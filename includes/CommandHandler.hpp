#pragma once

#include <string>
#include <vector>
#include <map>

// Forward declarations
class Server;
class Client;

class CommandHandler
{
    private:
        Server* _server;  // Référence vers le serveur

        // Utilitaires
        std::vector<std::string> split(const std::string& str, char delimiter);
        void sendResponse(int fd, const std::string& message);

    public:
        CommandHandler(Server* server);
        ~CommandHandler();

        // Fonction principale
        void processClientBuffer(int fd);
        void executeCommand(int fd, const std::string& command);
};