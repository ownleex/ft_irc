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
#include <cerrno>
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

        // std::vector<pollfd> : Choisi pour poll() car :
        // - poll() attend un tableau contigu de structures pollfd
        // - Accès par index rapide O(1) nécessaire pour poll()
        // - Insertion/suppression moins fréquentes que l'accès
        // - Compatible avec la fonction système poll(data(), size())
        std::vector<pollfd> _pollfds;

        // std::map<int, Client> : Choisi avec FD comme clé car :
        // - Recherche rapide O(log n) par file descriptor
        // - Association directe FD -> Client évite les recherches linéaires
        // - Insertion/suppression rapides O(log n)
        // - Tri automatique par FD pour debugging et affichage
        std::map<int, Client> _clients;

        // std::map<std::string, Channel> : Choisi avec nom de canal comme clé car :
        // - Recherche rapide O(log n) par nom de canal (#channel)
        // - Association directe nom -> Channel pour getChannel()
        // - Insertion/suppression rapides O(log n)
        // - Tri alphabétique automatique des noms de canaux
        // - Évite les doublons de canaux avec le même nom
        std::map<std::string, Channel> _channels;
        CommandHandler* _commandHandler;
        
        // SIGINT handling
        static volatile sig_atomic_t _stop;
        static void handleSignal(int signum);

        void initSocket();
        void newConnection();
        void clientData(int fd);

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
        void removeClient(int fd, const std::string& quitReason = "", bool voluntary = false);
};
