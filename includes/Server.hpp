#ifndef SERVER_HPP
#define SERVER_HPP
#include <string>
#include <iostream>
#include "cstdlib"
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

#endif