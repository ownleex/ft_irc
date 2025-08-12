#include <Server.hpp>

Server::Server(int port, const std::string &password) : _port(port), _password(password)
{
	std::cout << "Server open on port " << _port << std::endl;
}

Server::~Server()
{
	std::cout << "Server close !" << std::endl;
}

void Server::run_serv()
{
    while (1)
    {
        
    }
}