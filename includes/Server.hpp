#pragma once

#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include "Client.hpp"

class Server
{
    private:
        int _port;
        std::string _password;
        int _serverSocket;
        std::vector<pollfd> _pollfds;
        std::map<int, Client> _clients;

        void initSocket();
        void newConnection();
        void clientData();
        void removeClient();

    public:
        Server(int port, const std::string &password);
        ~Server();

        void run_serv();
};
