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
#include <csignal>
#include "Client.hpp"
#include "Channel.hpp"
#include "CommandHandler.hpp"

class Client;
class CommandHandler;
class Channel;

class Server
{
    private:
        int _port;
        std::string _password;
        int _serverSocket;
        std::vector<pollfd> _pollfds;
        std::map<int, Client> _clients;
        CommandHandler* _commandHandler;
        std::map<std::string, Channel> _channels;
        
        // SIGINT handling
        static volatile sig_atomic_t _stop;
        static void handleSignal(int signum);

        void initSocket();
        void newConnection();
        void clientData(int fd);
        void removeClient(int fd);

    public:
        Server(int port, const std::string &password);
        ~Server();

        void run_serv();
        
        // Getters
        std::map<int, Client>& getClients() { return _clients; }
        const std::string& getPassword() const { return _password; }

        Channel* getChannel(const std::string& name);
        Channel& createChannel(const std::string& name);
        void removeChannel(const std::string& name);
};
