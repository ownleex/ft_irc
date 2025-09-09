#include "CommandHandler.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include <iostream>
#include <sys/socket.h>

std::vector<std::string> CommandHandler::split(const std::string& str, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    for (size_t i = 0; i < str.length(); i++)
    {
        if (str[i] == delimiter)
        {
            if (!token.empty())
            {
                tokens.push_back(token);
                token.clear();
            }
        }
        else
            token += str[i];
    }
    if (!token.empty())
        tokens.push_back(token);
    return tokens;
}

void CommandHandler::sendResponse(int fd, const std::string& message)
{
    send(fd, message.c_str(), message.length(), 0);
    std::cout << "Sent to FD=" << fd << ": " << message;
}



void CommandHandler::handleHelp(int fd)
{
    if (!_server)
        return;

    std::string welcomeMsg =
    "Pour vous enregistrer, tapez les commandes suivantes :\r\n"
    "  PASS <password>\r\n"
    "  NICK <votre_pseudo>\r\n"
    "  USER <username> 0 * :<realname>\r\n"
    "Exemple : PASS secret | NICK John | USER john 0 * :John Doe\r\n";
    send(fd, welcomeMsg.c_str(), welcomeMsg.size(), 0);
}