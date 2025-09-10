#pragma once

#include <string>
#include <vector>
#include <map>

// Forward declarations
class Server;
class Client;
class Channel;

class CommandHandler
{
    private:
        Server* _server;  // Référence vers le serveur

        // Utilitaires
        std::vector<std::string> split(const std::string& str, char delimiter);
        void sendResponse(int fd, const std::string& message);

        // HELP
        void handleHelp(int fd, const std::vector<std::string>& params);

        // PASS
        void handlePass(int fd, const std::vector<std::string>& params);

        // NICK
        void handleNick(int fd, const std::vector<std::string>& params);
        bool isValidNickname(const std::string& nick);
        bool isNicknameInUse(const std::string& nick, int excludeFd = -1);

        // USER
        void handleUser(int fd, const std::vector<std::string>& params);

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

        // PRIVMSG
        void handlePrivmsg(int fd, const std::vector<std::string>& params);

        // QUIT
        void handleQuit(int fd, const std::vector<std::string>& params);

    public:
        CommandHandler(Server* server);
        ~CommandHandler();

        // Fonction principale
        void processClientBuffer(int fd);
        void executeCommand(int fd, const std::string& command);
};