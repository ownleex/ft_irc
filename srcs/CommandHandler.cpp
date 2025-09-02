#include "CommandHandler.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include <iostream>
#include <sys/socket.h>

CommandHandler::CommandHandler(Server* server) : _server(server)
{
}

CommandHandler::~CommandHandler()
{
}

void CommandHandler::processClientBuffer(int fd)
{
    if (!_server)
        return;

    std::map<int, Client>& clients = _server->getClients();
    std::map<int, Client>::iterator it = clients.find(fd);
    if (it == clients.end())
        return;

    std::string& buffer = const_cast<std::string&>(it->second.getBuffer());
    size_t pos = 0;
    
    // Chercher les commandes complètes (terminées par \n)
    while ((pos = buffer.find("\n")) != std::string::npos)
    {
        std::string command = buffer.substr(0, pos);
        buffer.erase(0, pos + 1);
        
        // Enlever \r si présent
        if (!command.empty() && command[command.length() - 1] == '\r')
            command.erase(command.length() - 1);
        
        if (!command.empty())
        {
            std::cout << "Processing command from FD=" << fd << ": " << command << std::endl;
            executeCommand(fd, command);
        }
    }
    
    // Remettre à jour le buffer du client
    it->second.setBuffer(buffer);
}

void CommandHandler::executeCommand(int fd, const std::string& command)
{
    std::vector<std::string> tokens = split(command, ' ');
    if (tokens.empty())
        return;

    std::string cmd = tokens[0];
    // Convertir en majuscules
    for (size_t i = 0; i < cmd.length(); i++)
        cmd[i] = std::toupper(cmd[i]);

    std::vector<std::string> params(tokens.begin() + 1, tokens.end());

    // 🎯 Pour l'instant, on fait juste du parsing et on affiche
    std::cout << "PARSED COMMAND from FD=" << fd << ": [" << cmd << "] with params: ";
    for (size_t i = 0; i < params.size(); i++)
    {
        std::cout << "[" << params[i] << "] ";
    }
    std::cout << std::endl;

    // TODO: ici on ajoutera les vraies commandes plus tard
    if (cmd == "PASS")
    {
        std::cout << "-> Will handle PASS command later" << std::endl;
    }
    else if (cmd == "NICK")
    {
        std::cout << "-> Will handle NICK command later" << std::endl;
    }
    else if (cmd == "USER")
    {
        std::cout << "-> Will handle USER command later" << std::endl;
    }
    else
    {
        std::cout << "-> Unknown command: " << cmd << std::endl;
    }
}

// Utilitaires
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