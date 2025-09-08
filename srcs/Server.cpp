#include <Server.hpp>
#include <cerrno>

volatile sig_atomic_t Server::_stop = 0;

void Server::handleSignal(int signum)
{
    if (signum == SIGINT)
        _stop = 1;
}

Server::Server(int port, const std::string &password) : _port(port), _password(password)
{
	std::cout << "Server open on port " << _port << std::endl;
    _commandHandler = new CommandHandler(this);
    // Installe le gestionnaire de signal (Ctrl+C)
    struct sigaction sa;
    std::memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &Server::handleSignal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0; // assure que les syscalls comme poll retournent EINTR
    sigaction(SIGINT, &sa, NULL);
    _stop = 0;
    initSocket();
}

Server::~Server()
{
    for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
        close(it->first);
    close(_serverSocket);
    delete _commandHandler;
	std::cout << "Server close !" << std::endl;
}

void Server::initSocket()
{
    _serverSocket = socket(AF_INET, SOCK_STREAM, 0); // En gros c'est la creation du socket avec addresse ip et protocol TCP
    // ca me renvoie un fd que linux uttilise pour identifier le reseau
    if (_serverSocket < 0)
        throw std::runtime_error("socket INIT failed");
    int opt = 1;
    setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in addr; // struct pour define l'addresse ip
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET; 
    addr.sin_addr.s_addr = INADDR_ANY; // accepte toute les connection
    addr.sin_port = htons(_port); // ca change le port en ordre reseau
    // en gros je donne des ordres a addresse ip sur les trois ligne du dessus
    if (bind(_serverSocket, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        throw std::runtime_error("bind failed");
    // je relie le port aux socket creer au dessus
    if (listen(_serverSocket, 10) < 0)
        throw std::runtime_error("socket mode failed");
    // je met le socket en mode server et le 10 c'est le nombre max de client qui peuvent attendre dans le file (modifiable)
    pollfd serverfd;
    serverfd.fd = _serverSocket; // on rajoute le socket creer
    serverfd.events = POLLIN; // on check si il y a des struct a lire
    _pollfds.push_back(serverfd); // on le rajoute a la liste des fds surveiller
}

void Server::run_serv()
{
    while (!_stop)
    {
        if (poll(_pollfds.data(), _pollfds.size(), -1) < 0)
        {
            if (errno == EINTR && _stop)
                break; // interrompu par SIGINT, on arrete
            throw std::runtime_error("poll failed");
        }
        for (size_t i = 0; i < _pollfds.size(); i++)
        {
            if (_pollfds[i].revents & POLLIN)
            {
                if (_pollfds[i].fd == _serverSocket)
                    newConnection();
                else
                    clientData(_pollfds[i].fd);
            }
        }
    }
    std::cout << "\nSIGINT received, shutting down..." << std::endl;
}

void Server::newConnection()
{
    sockaddr_in clientAddr;
    std::memset(&clientAddr, 0, sizeof(clientAddr));
    socklen_t len = sizeof(clientAddr);
    int clientFd = accept(_serverSocket, (struct sockaddr *) &clientAddr, &len);
    if (clientFd < 0)
    {
        if (errno == EINTR && _stop)
            return; // interrompu par SIGINT, on arrete
        std::cerr << "newConnection failed" << std::endl;
        return;
    }
    pollfd pfd;
    pfd.fd = clientFd;
    pfd.events = POLLIN;
    pfd.revents = 0;
    _pollfds.push_back(pfd);
    _clients.insert(std::make_pair(clientFd, Client(clientFd)));

    std::cout << "new connection from FD=" << clientFd << std::endl;
}

void Server::clientData(int fd)
{
    char buffer[512];
    ssize_t bytes = recv(fd, buffer, sizeof(buffer) - 1, 0); // -1 si on veux traiter le \0
    if (bytes < 0)
    {
        if (errno == EINTR)
            return; // interrompu par SIGINT, on arrete
    }
    if (bytes == 0)
    {
        std::cout << "Client disconnected from FD=" << fd << std::endl;
        removeClient(fd);
        return;
    }
    buffer[bytes] = '\0';

    std::map<int, Client>::iterator it = _clients.find(fd);
    if (it != _clients.end())
    {
        it->second.appendToBuffer(buffer);
        _commandHandler->processClientBuffer(fd);
    }
    else
    {
        std::cerr << "client not found" << std::endl;
        removeClient(fd);
    }
}

void Server::removeClient(int fd)
{
    close(fd);
    _clients.erase(fd);
    for (size_t i = 0; i < _pollfds.size(); i++)
    {
        if (_pollfds[i].fd == fd)
        {
            _pollfds.erase(_pollfds.begin() + i);
            break;
        }
    }
}

Channel* Server::getChannel(const std::string& name)
{
    std::map<std::string, Channel>::iterator it = _channels.find(name);
    if (it != _channels.end())
        return &(it->second);
    return NULL;
}

Channel& Server::createChannel(const std::string& name)
{
    std::pair<std::map<std::string, Channel>::iterator, bool> result;
    result = _channels.insert(std::make_pair(name, Channel(name)));
    return result.first->second;
}

void Server::removeChannel(const std::string& name)
{
    std::map<std::string, Channel>::iterator it = _channels.find(name);
    if (it != _channels.end() && it->second.isEmpty())
    {
        std::cout << "Channel " << name << " is empty, removing." << std::endl;
        _channels.erase(it);
    }
}
