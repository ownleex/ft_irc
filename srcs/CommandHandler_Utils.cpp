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



void CommandHandler::handleHelp(int fd, const std::vector<std::string>& params)
{
    if (!_server)
        return;

    if (params.empty())
    {
        std::string welcomeMsg =
        "Usage: HELPIRC <command>\r\n"
        "Commandes disponibles :\r\n"
        "  PASS, NICK, USER, KICK, INVITE, TOPIC, MODE, JOIN, PART, PRIVMSG, QUIT\r\n"
        "Exemple: HELPIRC JOIN\r\n";
        send(fd, welcomeMsg.c_str(), welcomeMsg.size(), 0);
        return;
    }

    std::string data = params[0];
    for (size_t i = 0; i < data.length(); i++)
        data[i] = std::toupper(data[i]);
    std::string welcomeMsg;
    if (data == "PASS")
    {
        welcomeMsg =
        "Provide the server password before registration :\r\n"
        "  PASS <password>\r\n";
        send(fd, welcomeMsg.c_str(), welcomeMsg.size(), 0);
    }
    else if (data == "NICK")
    {
        welcomeMsg =
        "Set or change your nickname (must be unique) :\r\n"
        "  NICK <nickname>\r\n";
        send(fd, welcomeMsg.c_str(), welcomeMsg.size(), 0);
    }
     else if (data == "USER")
    {
        welcomeMsg =
        "Define your username and real name (final step of registration) :\r\n"
        "  USER <username> 0 * :<realname>\r\n";
        send(fd, welcomeMsg.c_str(), welcomeMsg.size(), 0);
    }
     else if (data == "KICK")
    {
        welcomeMsg =
        "Remove a user from a channel with an optional reason :\r\n"
        "  KICK <#channel> <nick> [reason]\r\n";
        send(fd, welcomeMsg.c_str(), welcomeMsg.size(), 0);
    }
     else if (data == "INVITE")
    {
        welcomeMsg =
        "Invite a user to a channel (needed if it's invite-only) :\r\n"
        "  INVITE <nick> <#channel>\r\n";
        send(fd, welcomeMsg.c_str(), welcomeMsg.size(), 0);
    }
     else if (data == "TOPIC")
    {
        welcomeMsg =
        "View or set the topic of a channel :\r\n"
        "  TOPIC <#channel> <topic>\r\n";
        send(fd, welcomeMsg.c_str(), welcomeMsg.size(), 0);
    }
     else if (data == "MODE")
    {
        welcomeMsg =
        "Set channel modes (invite-only, password, user limit, operators) :\r\n"
        "  MODE <#channel> <modes> [params]\r\n"
        "Example : \r\n"
        "MODE #general +i            (set invite-only)\r\n"
        "MODE #general +k 123        (set password)\r\n"
        "MODE #general +l 10         (limit 10 users)\r\n"
        "MODE #general +o clem       (make JohnDoe operator)\r\n";
        send(fd, welcomeMsg.c_str(), welcomeMsg.size(), 0);
    }
     else if (data == "JOIN")
    {
        welcomeMsg =
        "Join (or create if it doesn't exist) a channel. Optionnally provide a password :\r\n"
        "  JOIN <#channel> [password]\r\n";
        send(fd, welcomeMsg.c_str(), welcomeMsg.size(), 0);
    }
     else if (data == "PART")
    {
        welcomeMsg =
        "Leave a channel, with an optionnal message :\r\n"
        "  PART <#channel> [message]\r\n";
        send(fd, welcomeMsg.c_str(), welcomeMsg.size(), 0);
    }
     else if (data == "PRIVMSG")
    {
        welcomeMsg =
        "Send a private message to a user or to a channel :\r\n"
        "  PRIVMSG <target> <message>\r\n";
        send(fd, welcomeMsg.c_str(), welcomeMsg.size(), 0);
    }
     else if (data == "QUIT")
    {
        welcomeMsg =
        "Disconnect from the server with an optional reason :\r\n"
        "  QUIT [massage]\r\n";
        send(fd, welcomeMsg.c_str(), welcomeMsg.size(), 0);
    }
    else
    {
        welcomeMsg =
        "Unknown command ! Try with :\r\n"
        "{PASS, NICK, USER, KICK, INVITE, TOPIC, MODE, JOIN, PART, PRIVMSG, QUIT}\r\n";
        send(fd, welcomeMsg.c_str(), welcomeMsg.size(), 0);
    }
}