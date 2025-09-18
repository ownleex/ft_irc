#pragma once

#include <string>
#include <vector>
#include <map>

class Server;
class Client;
class Channel;

class CommandHandler
{
    private:
        Server* _server;  // Référence vers le serveur

        /// CommandHandler_Utils.cpp
        // HELP
        void handleHelp(int fd, const std::vector<std::string>& params);
        void sendResponse(int fd, const std::string& message);
        // Split
        // std::vector<std::string> : Choisi pour split() car :
        // - Ordre des tokens important (paramètres de commande séquentiels)
        // - Accès par index nécessaire pour parser les commandes IRC
        // - Nombre variable de paramètres selon la commande
        // - Itération séquentielle fréquente
        std::vector<std::string> split(const std::string& str, char delimiter);


        /// CommandHandler_Auth.cpp
        // PASS
        void handlePass(int fd, const std::vector<std::string>& params);
        // NICK
        void handleNick(int fd, const std::vector<std::string>& params);
        bool isValidNickname(const std::string& nick);
        bool isNicknameInUse(const std::string& nick, int excludeFd = -1);
        // USER
        void handleUser(int fd, const std::vector<std::string>& params);


        /// CommandHandler_Channel.cpp
        // JOIN
        void handleJoin(int fd, const std::vector<std::string>& params);
        void sendNamesList(int fd, const std::string& channelName, Channel* channel);
        // MODE
        void handleMode(int fd, const std::vector<std::string>& params);
        bool isValidModeChar(char mode);
        void applyChannelMode(Channel* channel, char mode, bool add, const std::string& param);
        // KICK
        void handleKick(int fd, const std::vector<std::string>& params);
        // TOPIC
        void handleTopic(int fd, const std::vector<std::string>& params);
        // INVITE
        void handleInvite(int fd, const std::vector<std::string>& params);
        // PART
        void handlePart(int fd, const std::vector<std::string>& params);


        /// CommandHandler_Messages.cpp
        // PRIVMSG
        void handlePrivmsg(int fd, const std::vector<std::string>& params);
        // QUIT
        void handleQuit(int fd, const std::vector<std::string>& params);

    public:

        /// CommandHanler_Core.cpp
        CommandHandler(Server* server);
        ~CommandHandler();

        // Fonction principale
        void processClientBuffer(int fd);
        void executeCommand(int fd, const std::string& command);
};