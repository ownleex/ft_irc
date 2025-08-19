#include <Server.hpp>

Server::Server(int port, const std::string &password) : _port(port), _password(password)
{
	std::cout << "Server open on port " << _port << std::endl;
}

Server::~Server()
{
	std::cout << "Server close !" << std::endl;
}

void Server::initSocket()
{
    _serverSocket = socket(AF_INET, SOCK_STREAM, 0); // En gros c'est la creation du socket avec addresse ip et protocol TCP
    // ca me renvoie un fd que linux uttilise pour identifier le reseau
    if (_serverSocket < 0)
        throw std::runtime_error("socket INIT failed");
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
    while (true)
    {
        if (poll(_pollfds.data(), _pollfds.size(), -1) < 0)
            throw std::runtime_error("poll failed");
        for (size_t i = 0; i < _pollfds.size(); i++)
        {
            if (_pollfds[i].revents & POLLIN)
            {
                if (_pollfds[i].fd == _serverSocket)
                    // newConnection();
                else
                    // clientData();
            }
        }
    }
}