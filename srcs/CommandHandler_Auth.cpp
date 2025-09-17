#include "CommandHandler.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include <iostream>

void CommandHandler::handlePass(int fd, const std::vector<std::string>& params)
{
    if (!_server)
        return;

    std::map<int, Client>& clients = _server->getClients();
    std::map<int, Client>::iterator clientIt = clients.find(fd);
    if (clientIt == clients.end())
        return;

    Client& client = clientIt->second;

    // Vérifier si le client est déjà authentifié
    if (client.isAuthenticated())
    {
        // ERR_ALREADYREGISTERED (462) 
        sendResponse(fd, ":ircserv 462 * :You may not reregister\r\n");
        return;
    }

    // Vérifier qu'il y a bien un paramètre (le mot de passe)
    if (params.empty())
    {
        // ERR_NEEDMOREPARAMS (461)
        sendResponse(fd, ":ircserv 461 * PASS :Not enough parameters\r\n");
        return;
    }

    std::string providedPassword = params[0];
    
    // Récupérer le mot de passe du serveur
    std::string serverPassword = _server->getPassword();
    
    // Comparer avec le mot de passe fourni
    if (providedPassword != serverPassword)
    {
        // ERR_PASSWDMISMATCH (464)
        sendResponse(fd, ":ircserv 464 * :Password incorrect\r\n");
        std::cout << "Client FD=" << fd << " provided incorrect password" << std::endl;
        return;
    }

    // Mot de passe correct
    client.setAuthenticated(true);
    std::cout << "Client FD=" << fd << " authenticated successfully" << std::endl;
    
    // Optionnel : envoyer une confirmation (pas obligatoire selon RFC)
    // sendResponse(fd, "NOTICE * :Password accepted\r\n");
}

void CommandHandler::handleNick(int fd, const std::vector<std::string>& params)
{
    if (!_server)
        return;

    std::map<int, Client>& clients = _server->getClients();
    std::map<int, Client>::iterator clientIt = clients.find(fd);
    if (clientIt == clients.end())
        return;

    Client& client = clientIt->second;

    // Vérifier qu'il y a bien un paramètre (le nickname)
    if (params.empty())
    {
        // ERR_NONICKNAMEGIVEN (431)
        std::string nick = client.getNickname().empty() ? "*" : client.getNickname();
        sendResponse(fd, ":ircserv 431 " + nick + " :No nickname given\r\n");
        return;
    }

    std::string newNick = params[0];

    // Vérifier que le nickname est valide (commence par lettre, contient lettres/chiffres/_-)
    if (!isValidNickname(newNick))
    {
        // ERR_ERRONEUSNICKNAME (432)
        std::string nick = client.getNickname().empty() ? "*" : client.getNickname();
        sendResponse(fd, ":ircserv 432 " + nick + " " + newNick + " :Erroneous nickname\r\n");
        return;
    }

    // Vérifier que le nickname n'est pas déjà utilisé
    if (isNicknameInUse(newNick, fd))
    {
        // ERR_NICKNAMEINUSE (433)
        std::string nick = client.getNickname().empty() ? "*" : client.getNickname();
        sendResponse(fd, ":ircserv 433 " + nick + " " + newNick + " :Nickname is already in use\r\n");
        return;
    }

    // Sauvegarder l'ancien nickname pour les notifications
    std::string oldNick = client.getNickname();
    
    // Définir le nouveau nickname
    client.setNickname(newNick);
    
    if (!oldNick.empty())
    {
        // Si le client avait déjà un nickname, notifier le changement
        std::string username = client.getUsername();
        std::string hostname = client.getHostname();
        std::string fullMask = oldNick + "!" + username + "@" + hostname;
        std::string nickMsg = ":" + fullMask + " NICK " + newNick + "\r\n";
        
        // Collecter tous les clients qui doivent être notifiés
        std::set<int> clientsToNotify;
        
        // Ajouter le client lui-même
        clientsToNotify.insert(fd);
        
        // Parcourir tous les canaux où le client est présent
        const std::set<std::string>& clientChannels = client.getChannels();
        for (std::set<std::string>::const_iterator channelIt = clientChannels.begin(); 
             channelIt != clientChannels.end(); ++channelIt)
        {
            const std::string& channelName = *channelIt;
            Channel* channel = _server->getChannel(channelName);
            
            if (channel)
            {
                // Ajouter tous les membres de ce canal à la liste des clients à notifier
                std::vector<int> members = channel->getAllMembers();
                for (size_t i = 0; i < members.size(); i++)
                {
                    clientsToNotify.insert(members[i]);
                }
            }
        }
        
        // Envoyer la notification NICK à tous les clients concernés
        for (std::set<int>::iterator it = clientsToNotify.begin(); it != clientsToNotify.end(); ++it)
        {
            sendResponse(*it, nickMsg);
        }
        
        std::cout << "Client FD=" << fd << " changed nickname from " << oldNick << " to " << newNick << std::endl;
        std::cout << "Notified " << clientsToNotify.size() << " clients of nickname change" << std::endl;
    }
    else
    {
        std::cout << "Client FD=" << fd << " set nickname to " << newNick << std::endl;
    }

    // Vérifier si le client peut maintenant être considéré comme enregistré
    // (authentifié + nickname + username définis)
    if (client.isAuthenticated() && !client.getNickname().empty() && 
        !client.getUsername().empty() && !client.isRegistered())
    {
        client.setRegistered(true);
        
        // Envoyer les messages de bienvenue
        sendResponse(fd, ":ircserv 001 " + newNick + " :Welcome to the Internet Relay Network " + newNick + "\r\n");
        sendResponse(fd, ":ircserv 002 " + newNick + " :Your host is ircserv, running version 1.0\r\n");
        sendResponse(fd, ":ircserv 003 " + newNick + " :This server was created today\r\n");
        sendResponse(fd, ":ircserv 004 " + newNick + " ircserv 1.0 o o\r\n");
        
        std::cout << "Client FD=" << fd << " (" << newNick << ") is now fully registered" << std::endl;
    }
}

bool CommandHandler::isValidNickname(const std::string& nick)
{
    if (nick.empty()) // RFC 1459: max 9 caractères
        return false;
    
    // Premier caractère doit être une lettre
    if (!std::isalpha(nick[0]))
        return false;
    
    // Les autres caractères peuvent être lettres, chiffres, - ou _
    for (size_t i = 1; i < nick.length(); i++)
    {
        char c = nick[i];
        if (!std::isalnum(c) && c != '-' && c != '_')
            return false;
    }
    
    return true;
}

bool CommandHandler::isNicknameInUse(const std::string& nick, int excludeFd)
{
    if (!_server)
        return false;
        
    std::map<int, Client>& clients = _server->getClients();
    
    for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
    {
        if (it->first != excludeFd && it->second.getNickname() == nick)
            return true;
    }
    
    return false;
}

void CommandHandler::handleUser(int fd, const std::vector<std::string>& params)
{
    if (!_server)
        return;

    std::map<int, Client>& clients = _server->getClients();
    std::map<int, Client>::iterator clientIt = clients.find(fd);
    if (clientIt == clients.end())
        return;

    Client& client = clientIt->second;

    // Vérifier que le client est authentifié
    if (!client.isAuthenticated())
    {
        sendResponse(fd, ":ircserv 451 * :You have not registered\r\n");
        return;
    }

    // Vérifier qu'il y a au moins 4 paramètres
    if (params.size() < 4)
    {
        // ERR_NEEDMOREPARAMS (461)
        std::string nick = client.getNickname().empty() ? "*" : client.getNickname();
        sendResponse(fd, ":ircserv 461 " + nick + " USER :Not enough parameters\r\n");
        return;
    }

    // Vérifier que le client n'est pas déjà enregistré
    if (client.isRegistered())
    {
        // ERR_ALREADYREGISTERED (462)
        sendResponse(fd, ":ircserv 462 " + client.getNickname() + " :You may not reregister\r\n");
        return;
    }

    // Extraire les paramètres
    std::string username = params[0];
    // params[1] est hostname (ignoré, on utilise localhost)
    // params[2] est servername (ignoré)
    std::string realname = params[3];

    // Le realname peut commencer par ':' et contenir des espaces
    if (realname[0] == ':')
        realname = realname.substr(1);
    
    // Reconstituer le realname complet si il y a des espaces
    for (size_t i = 4; i < params.size(); i++)
        realname += " " + params[i];

    // Définir les informations du client
    client.setUser(username);
    client.setRealname(realname);
    client.setHostname("localhost"); // On peut utiliser localhost ou l'IP réelle

    std::cout << "Client FD=" << fd << " set user info: " << username << " / " << realname << std::endl;

    // Vérifier si le client peut maintenant être considéré comme complètement enregistré
    // (authentifié + nickname + username définis)
    if (client.isAuthenticated() && !client.getNickname().empty() && 
        !client.getUsername().empty() && !client.isRegistered())
    {
        client.setRegistered(true);
        
        std::string nick = client.getNickname();
        std::string userhost = nick + "!" + username + "@" + client.getHostname();
        
        // Envoyer les messages de bienvenue (RPL_WELCOME, etc.)
        sendResponse(fd, ":ircserv 001 " + nick + " :Welcome to the Internet Relay Network " + userhost + "\r\n");
        sendResponse(fd, ":ircserv 002 " + nick + " :Your host is ircserv, running version 1.0\r\n");
        sendResponse(fd, ":ircserv 003 " + nick + " :This server was created today\r\n");
        sendResponse(fd, ":ircserv 004 " + nick + " ircserv 1.0 o o\r\n");
        
        std::cout << "Client FD=" << fd << " (" << nick << ") is now fully registered" << std::endl;
    }
}