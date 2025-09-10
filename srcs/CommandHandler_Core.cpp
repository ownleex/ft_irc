#include "CommandHandler.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include <iostream>

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
            
            // Si le client a été supprimé, arrêter immédiatement
            if (clients.find(fd) == clients.end())
            {
                std::cout << "Client FD=" << fd << " was removed during command execution, stopping processing" << std::endl;
                return;
            }
        }
    }
    
    // S'assurer que le client existe encore avant de mettre à jour le buffer
    it = clients.find(fd);
    if (it != clients.end())
    {
        it->second.setBuffer(buffer);
    }
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

    // Parsing et on affiche
    std::cout << "PARSED COMMAND from FD=" << fd << ": [" << cmd << "] with params: ";
    for (size_t i = 0; i < params.size(); i++)
    {
        std::cout << "[" << params[i] << "] ";
    }
    std::cout << std::endl;

    if (cmd == "PASS")
    {
        handlePass(fd, params);
    }
    else if (cmd == "HELPIRC")
    {
        handleHelp(fd, params);
    }
    else if (cmd == "NICK")
    {
        handleNick(fd, params);
    }
    else if (cmd == "USER")
    {
        handleUser(fd, params);
    }
    else if (cmd == "JOIN")
    {
        handleJoin(fd, params);
    }
    else if (cmd == "PRIVMSG")
    {
        handlePrivmsg(fd, params);
    }
    else if (cmd == "MODE")
    {
        handleMode(fd, params);
    }
    else if (cmd == "KICK")
    {
        handleKick(fd, params);
    }
    else if (cmd == "TOPIC")
    {
        handleTopic(fd, params);
    }
    else if (cmd == "INVITE")
    {
        handleInvite(fd, params);
    }
    else if (cmd == "PART")
    {
        handlePart(fd, params);
    }
    else if (cmd == "QUIT")
    {
        handleQuit(fd, params);
    }
    else
    {
        std::cout << "-> Unknown command: " << cmd << std::endl;
    }
}