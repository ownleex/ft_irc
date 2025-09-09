#include "CommandHandler.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include <iostream>

void CommandHandler::handlePrivmsg(int fd, const std::vector<std::string>& params)
{
    if (!_server)
        return;

    std::map<int, Client>& clients = _server->getClients();
    std::map<int, Client>::iterator clientIt = clients.find(fd);
    if (clientIt == clients.end())
        return;

    Client& client = clientIt->second;

    // Vérifier que le client est enregistré
    if (!client.isRegistered())
    {
        sendResponse(fd, ":ircserv 451 * :You have not registered\r\n");
        return;
    }

    // Vérifier qu'il y a au moins 2 paramètres (destinataire et message)
    if (params.size() < 2)
    {
        // ERR_NEEDMOREPARAMS (461)
        sendResponse(fd, ":ircserv 461 " + client.getNickname() + " PRIVMSG :Not enough parameters\r\n");
        return;
    }

    std::string target = params[0];
    std::string message = params[1];

    // Si le message commence par ':', l'enlever
    if (!message.empty() && message[0] == ':')
        message = message.substr(1);
    
    // Reconstituer le message complet si il y a des espaces
    for (size_t i = 2; i < params.size(); i++)
        message += " " + params[i];

    // Vérifier si la cible est vide
    if (target.empty())
    {
        // ERR_NORECIPIENT (411)
        sendResponse(fd, ":ircserv 411 " + client.getNickname() + " :No recipient given (PRIVMSG)\r\n");
        return;
    }

    // Vérifier si le message est vide
    if (message.empty())
    {
        // ERR_NOTEXTTOSEND (412)
        sendResponse(fd, ":ircserv 412 " + client.getNickname() + " :No text to send\r\n");
        return;
    }

    std::string nick = client.getNickname();
    std::string user = client.getUsername();
    std::string host = client.getHostname();
    std::string fullMask = nick + "!" + user + "@" + host;

    // 🎯 CASE 1: Message vers un CANAL (commence par # ou &)
    if (target[0] == '#' || target[0] == '&')
    {
        Channel* channel = _server->getChannel(target);
        
        if (!channel)
        {
            // ERR_NOSUCHCHANNEL (403)
            sendResponse(fd, ":ircserv 403 " + nick + " " + target + " :No such channel\r\n");
            return;
        }

        // Vérifier que l'expéditeur est membre du canal
        if (!channel->isMember(fd))
        {
            // ERR_CANNOTSENDTOCHAN (404)
            sendResponse(fd, ":ircserv 404 " + nick + " " + target + " :Cannot send to channel\r\n");
            return;
        }

        // Construire le message à diffuser
        std::string privmsgToChannel = ":" + fullMask + " PRIVMSG " + target + " :" + message + "\r\n";

        // Envoyer à TOUS les membres du canal SAUF l'expéditeur
        std::vector<int> members = channel->getAllMembers();
        int messagesSent = 0;
        
        for (size_t i = 0; i < members.size(); i++)
        {
            if (members[i] != fd) // Ne pas renvoyer à l'expéditeur
            {
                sendResponse(members[i], privmsgToChannel);
                messagesSent++;
            }
        }

        std::cout << "PRIVMSG from " << nick << " to channel " << target 
                  << " (" << messagesSent << " recipients): " << message << std::endl;
    }
    // 🎯 CASE 2: Message vers un UTILISATEUR
    else
    {
        // Chercher l'utilisateur destinataire
        int targetFd = -1;
        
        for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
        {
            if (it->second.getNickname() == target)
            {
                targetFd = it->first;
                break;
            }
        }

        if (targetFd == -1)
        {
            // ERR_NOSUCHNICK (401)
            sendResponse(fd, ":ircserv 401 " + nick + " " + target + " :No such nick/channel\r\n");
            return;
        }

        // Construire le message privé
        std::string privmsgToUser = ":" + fullMask + " PRIVMSG " + target + " :" + message + "\r\n";

        // Envoyer le message au destinataire
        sendResponse(targetFd, privmsgToUser);

        std::cout << "PRIVMSG from " << nick << " to user " << target << ": " << message << std::endl;
    }
}



void CommandHandler::handleQuit(int fd, const std::vector<std::string>& params)
{
    if (!_server)
        return;

    std::map<int, Client>& clients = _server->getClients();
    std::map<int, Client>::iterator clientIt = clients.find(fd);
    if (clientIt == clients.end())
        return;

    Client& client = clientIt->second;

    // Construire le message de quit
    std::string quitMessage = "Client Quit";
    if (!params.empty())
    {
        quitMessage = params[0];
        // Si le message commence par ':', l'enlever
        if (quitMessage[0] == ':')
            quitMessage = quitMessage.substr(1);
        
        // Reconstituer le message complet si il y a des espaces
        for (size_t i = 1; i < params.size(); i++)
            quitMessage += " " + params[i];
    }

    std::string nickname = client.getNickname();
    std::string username = client.getUsername();
    std::string hostname = client.getHostname();

    // Construire le prefix complet (nick!user@host)
    std::string prefix;
    if (!nickname.empty())
    {
        prefix = nickname;
        if (!username.empty())
            prefix += "!" + username;
        if (!hostname.empty())
            prefix += "@" + hostname;
    }
    else
    {
        prefix = "*"; // Fallback si pas de nickname
    }

    std::cout << "Client FD=" << fd << " (" << (nickname.empty() ? "unknown" : nickname) 
              << ") disconnecting: " << quitMessage << std::endl;

    // 📢 NOUVEAU : Notifier tous les canaux où le client était présent
    if (client.isRegistered() && !nickname.empty())
    {
        std::string quitNotification = ":" + prefix + " QUIT :Quit: " + quitMessage + "\r\n";
        
        // Obtenir la liste des canaux du client
        const std::set<std::string>& clientChannels = client.getChannels();
        std::set<int> notifiedClients; // Pour éviter de notifier plusieurs fois le même client
        
        for (std::set<std::string>::const_iterator channelIt = clientChannels.begin(); 
             channelIt != clientChannels.end(); ++channelIt)
        {
            const std::string& channelName = *channelIt;
            Channel* channel = _server->getChannel(channelName);
            
            if (channel)
            {
                std::cout << "Notifying channel " << channelName << " of " << nickname << "'s quit" << std::endl;
                
                // Obtenir tous les membres du canal
                std::vector<int> members = channel->getAllMembers();
                
                for (size_t i = 0; i < members.size(); i++)
                {
                    int memberFd = members[i];
                    
                    // Ne pas notifier le client qui quitte, et éviter les doublons
                    if (memberFd != fd && notifiedClients.find(memberFd) == notifiedClients.end())
                    {
                        sendResponse(memberFd, quitNotification);
                        notifiedClients.insert(memberFd);
                    }
                }
                
                // Retirer le client du canal
                channel->removeMember(fd);
                
                // Supprimer le canal s'il devient vide
                if (channel->isEmpty())
                {
                    _server->removeChannel(channelName);
                }
            }
        }
        
        // Nettoyer la liste des canaux du client
        // (Ceci sera fait automatiquement quand le client sera supprimé)
    }

    // Envoyer la confirmation de QUIT au client lui-même
    if (client.isRegistered())
    {
        std::string quitResponse = ":" + prefix + " QUIT :Quit: " + quitMessage + "\r\n";
        send(fd, quitResponse.c_str(), quitResponse.length(), 0);
    }

    // Fermer la connexion
    close(fd);
    
    // Note: Le client sera automatiquement retiré de _pollfds et _clients 
    // par Server::removeClient() qui sera appelé dans la boucle principale
    // quand recv() retournera 0 ou quand on détecte que la connexion est fermée
}