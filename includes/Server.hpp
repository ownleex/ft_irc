#pragma once

#include <string>
#include <iostream>
#include "Client.hpp"

class Server
{
    private:
        int _port;
        std::string _password;

    public:
        Server(int port, const std::string &password);
        ~Server();

        void run_serv();
};
