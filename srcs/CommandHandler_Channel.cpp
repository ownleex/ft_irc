#include "CommandHandler.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include <iostream>

void CommandHandler::handleJoin(int fd, const std::vector<std::string>& params)
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

    // Vérifier qu'il y a au moins un paramètre (nom du canal)
    if (params.empty())
    {
        // ERR_NEEDMOREPARAMS (461)
        sendResponse(fd, ":ircserv 461 " + client.getNickname() + " JOIN :Not enough parameters\r\n");
        return;
    }

    // Parser les canaux et mots de passe
    std::string channelList = params[0];
    std::string passwordList = params.size() > 1 ? params[1] : "";

    std::vector<std::string> channels = split(channelList, ',');
    std::vector<std::string> passwords = split(passwordList, ',');

    // Joindre chaque canal
    for (size_t i = 0; i < channels.size(); i++)
    {
        std::string channelName = channels[i];
        std::string password = (i < passwords.size()) ? passwords[i] : "";

        // Vérifier que le nom du canal est valide (commence par # ou &)
        if (channelName.empty() || (channelName[0] != '#' && channelName[0] != '&'))
        {
            // ERR_NOSUCHCHANNEL (403)
            sendResponse(fd, ":ircserv 403 " + client.getNickname() + " " + channelName + " :No such channel\r\n");
            continue;
        }

        // Vérifier si le client est déjà dans le canal
        Channel* channel = _server->getChannel(channelName);
        if (channel && channel->isMember(fd))
        {
            // Le client est déjà dans le canal, on ignore
            continue;
        }

        // Créer le canal s'il n'existe pas
        if (!channel)
        {
            channel = &_server->createChannel(channelName);
            std::cout << "Created new channel: " << channelName << std::endl;
        }

        // Vérifier si le client peut rejoindre le canal
        if (!channel->canJoin(fd, password))
        {
            // Vérifier la raison spécifique
            if (channel->getUserLimit() > 0 && channel->getMemberCount() >= static_cast<size_t>(channel->getUserLimit()))
            {
                // ERR_CHANNELISFULL (471)
                sendResponse(fd, ":ircserv 471 " + client.getNickname() + " " + channelName + " :Cannot join channel (+l)\r\n");
            }
            else if (channel->isInviteOnly() && !channel->isInvited(fd))
            {
                // ERR_INVITEONLYCHAN (473)
                sendResponse(fd, ":ircserv 473 " + client.getNickname() + " " + channelName + " :Cannot join channel (+i)\r\n");
            }
            else if (channel->hasPassword() && password != channel->getPassword())
            {
                // ERR_BADCHANNELKEY (475)
                sendResponse(fd, ":ircserv 475 " + client.getNickname() + " " + channelName + " :Cannot join channel (+k)\r\n");
            }
            continue;
        }

        // Ajouter le client au canal
        if (channel->addMember(fd))
        {
            // 🎯 LIGNE CRUCIALE AJOUTÉE :
            client.joinChannel(channelName);
            
            // Supprimer l'invitation si elle existait
            channel->removeInvited(fd);

            std::string nick = client.getNickname();
            std::string user = client.getUsername();
            std::string host = client.getHostname();
            std::string fullMask = nick + "!" + user + "@" + host;

            // Envoyer JOIN à TOUS les membres du canal (y compris le nouveau)
            std::string joinMsg = ":" + fullMask + " JOIN " + channelName + "\r\n";
            std::vector<int> members = channel->getAllMembers();
            
            for (size_t j = 0; j < members.size(); j++)
            {
                sendResponse(members[j], joinMsg);
            }

            std::cout << "Client " << nick << " joined channel " << channelName << std::endl;

            // Les messages suivants vont SEULEMENT au nouveau client
            
            // Envoyer le topic du canal s'il existe
            if (!channel->getTopic().empty())
            {
                sendResponse(fd, ":ircserv 332 " + nick + " " + channelName + " :" + channel->getTopic() + "\r\n");
            }
            else
            {
                sendResponse(fd, ":ircserv 331 " + nick + " " + channelName + " :No topic is set\r\n");
            }

            // Envoyer la liste des utilisateurs SEULEMENT au nouveau client
            sendNamesList(fd, channelName, channel);
        }
    }
}

// Fonction utilitaire pour envoyer la liste des noms
void CommandHandler::sendNamesList(int fd, const std::string& channelName, Channel* channel)
{
    if (!_server || !channel)
        return;

    std::map<int, Client>& clients = _server->getClients();
    std::map<int, Client>::iterator clientIt = clients.find(fd);
    if (clientIt == clients.end())
        return;

    std::string nick = clientIt->second.getNickname();
    std::string namesList = "";
    
    std::vector<int> members = channel->getAllMembers();
    
    // Construire la liste des noms
    for (size_t i = 0; i < members.size(); i++)
    {
        std::map<int, Client>::iterator memberIt = clients.find(members[i]);
        if (memberIt != clients.end())
        {
            if (!namesList.empty())
                namesList += " ";
            
            // Ajouter @ pour les opérateurs, + pour les voices (si implémenté)
            if (channel->isOperator(members[i]))
                namesList += "@";
            
            namesList += memberIt->second.getNickname();
        }
    }

    // Debug pour voir ce qu'on envoie
    std::cout << "Sending NAMES to " << nick << " for " << channelName << ": " << namesList << std::endl;

    // RPL_NAMREPLY (353) - IMPORTANT: le = indique un canal public
    sendResponse(fd, ":ircserv 353 " + nick + " = " + channelName + " :" + namesList + "\r\n");

    // RPL_ENDOFNAMES (366)
    sendResponse(fd, ":ircserv 366 " + nick + " " + channelName + " :End of /NAMES list\r\n");
}

void CommandHandler::handleMode(int fd, const std::vector<std::string>& params)
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

    // Vérifier qu'il y a au moins un paramètre (nom du canal)
    if (params.empty())
    {
        // ERR_NEEDMOREPARAMS (461)
        sendResponse(fd, ":ircserv 461 " + client.getNickname() + " MODE :Not enough parameters\r\n");
        return;
    }

    std::string target = params[0];
    std::string nick = client.getNickname();

    // Pour l'instant, on ne gère que les modes de canaux (commencent par # ou &)
    if (target[0] != '#' && target[0] != '&')
    {
        // ERR_NOSUCHNICK (401) - pour les utilisateurs, pas implémenté
        sendResponse(fd, ":ircserv 401 " + nick + " " + target + " :No such nick/channel\r\n");
        return;
    }

    // Vérifier que le canal existe
    Channel* channel = _server->getChannel(target);
    if (!channel)
    {
        // ERR_NOSUCHCHANNEL (403)
        sendResponse(fd, ":ircserv 403 " + nick + " " + target + " :No such channel\r\n");
        return;
    }

    // Vérifier que le client est membre du canal
    if (!channel->isMember(fd))
    {
        // ERR_NOTONCHANNEL (442)
        sendResponse(fd, ":ircserv 442 " + nick + " " + target + " :You're not on that channel\r\n");
        return;
    }

    // Si pas de modes spécifiés, afficher les modes actuels
    if (params.size() == 1)
    {
        std::string modeString = channel->getModeString();
        // RPL_CHANNELMODEIS (324)
        sendResponse(fd, ":ircserv 324 " + nick + " " + target + " " + modeString + "\r\n");
        return;
    }

    // Vérifier que l'utilisateur est opérateur pour modifier les modes
    if (!channel->isOperator(fd))
    {
        // ERR_CHANOPRIVSNEEDED (482)
        sendResponse(fd, ":ircserv 482 " + nick + " " + target + " :You're not channel operator\r\n");
        return;
    }

    // Parser les modes
    std::string modestring = params[1];
    std::vector<std::string> modeParams;
    
    // Collecter les paramètres supplémentaires
    for (size_t i = 2; i < params.size(); i++)
        modeParams.push_back(params[i]);

    // Variables pour tracker les changements
    std::string appliedModes = "";
    bool adding = true;
    size_t paramIndex = 0;

    for (size_t i = 0; i < modestring.length(); i++)
    {
        char c = modestring[i];
        
        if (c == '+')
        {
            adding = true;
            continue;
        }
        else if (c == '-')
        {
            adding = false;
            continue;
        }

        // Vérifier que c'est un mode valide
        if (!isValidModeChar(c))
        {
            // ERR_UNKNOWNMODE (472)
            sendResponse(fd, ":ircserv 472 " + nick + " " + std::string(1, c) + " :is unknown mode char to me\r\n");
            continue;
        }

        std::string param = "";
        
        // Certains modes nécessitent des paramètres
        if ((c == 'k' && adding) || (c == 'l' && adding) || c == 'o')
        {
            if (paramIndex < modeParams.size())
            {
                param = modeParams[paramIndex];
                paramIndex++;
            }
            else
            {
                // Paramètre manquant
                continue;
            }
        }

        // Appliquer le mode
        applyChannelMode(channel, c, adding, param);
        
        // Ajouter à la liste des modes appliqués
        if (appliedModes.empty())
            appliedModes += (adding ? "+" : "-");
        else if ((adding && appliedModes[appliedModes.length()-1] == '-') ||
                 (!adding && appliedModes[appliedModes.length()-1] == '+'))
            appliedModes += (adding ? "+" : "-");
        
        appliedModes += c;
    }

    // Si des modes ont été appliqués, notifier le canal
    if (!appliedModes.empty())
    {
        std::string user = client.getUsername();
        std::string host = client.getHostname();
        std::string fullMask = nick + "!" + user + "@" + host;
        
        std::string modeMsg = ":" + fullMask + " MODE " + target + " " + appliedModes;
        
        // Ajouter les paramètres si nécessaire
        if (!modeParams.empty())
        {
            for (size_t i = 0; i < modeParams.size() && i < paramIndex; i++)
                modeMsg += " " + modeParams[i];
        }
        modeMsg += "\r\n";

        // Envoyer à tous les membres du canal
        std::vector<int> members = channel->getAllMembers();
        for (size_t i = 0; i < members.size(); i++)
        {
            sendResponse(members[i], modeMsg);
        }

        std::cout << "Mode changed in " << target << " by " << nick << ": " << appliedModes << std::endl;
    }
}

bool CommandHandler::isValidModeChar(char mode)
{
    // Modes supportés : i (invite-only), t (topic restricted), k (key), l (limit), o (operator)
    return (mode == 'i' || mode == 't' || mode == 'k' || mode == 'l' || mode == 'o');
}

void CommandHandler::applyChannelMode(Channel* channel, char mode, bool add, const std::string& param)
{
    if (!channel || !_server)
        return;

    std::map<int, Client>& clients = _server->getClients();

    switch (mode)
    {
        case 'i': // invite-only
            channel->setInviteOnly(add);
            break;
            
        case 't': // topic restricted
            channel->setTopicRestricted(add);
            break;
            
        case 'k': // key (password)
            if (add && !param.empty())
            {
                channel->setPassword(param);
            }
            else if (!add)
            {
                channel->setPassword("");
            }
            break;
            
        case 'l': // user limit
            if (add && !param.empty())
            {
                int limit = std::atoi(param.c_str());
                if (limit > 0)
                    channel->setUserLimit(limit);
            }
            else if (!add)
            {
                channel->setUserLimit(0);
            }
            break;
            
        case 'o': // operator privilege
            if (!param.empty())
            {
                // Trouver l'utilisateur cible
                int targetFd = -1;
                for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
                {
                    if (it->second.getNickname() == param)
                    {
                        targetFd = it->first;
                        break;
                    }
                }
                
                if (targetFd != -1 && channel->isMember(targetFd))
                {
                    if (add)
                        channel->addOperator(targetFd);
                    else
                        channel->removeOperator(targetFd);
                }
            }
            break;
    }
}

void CommandHandler::handleKick(int fd, const std::vector<std::string>& params)
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

    // Vérifier qu'il y a au moins 2 paramètres (canal et utilisateur)
    if (params.size() < 2)
    {
        // ERR_NEEDMOREPARAMS (461)
        sendResponse(fd, ":ircserv 461 " + client.getNickname() + " KICK :Not enough parameters\r\n");
        return;
    }

    std::string channelName = params[0];
    std::string targetNick = params[1];
    std::string kickReason = "No reason given";

    // Construire la raison du kick (optionnelle)
    if (params.size() >= 3)
    {
        kickReason = params[2];
        // Si la raison commence par ':', l'enlever
        if (!kickReason.empty() && kickReason[0] == ':')
            kickReason = kickReason.substr(1);
        
        // Reconstituer la raison complète si il y a des espaces
        for (size_t i = 3; i < params.size(); i++)
            kickReason += " " + params[i];
    }

    std::string nick = client.getNickname();

    // Vérifier que le canal existe
    Channel* channel = _server->getChannel(channelName);
    if (!channel)
    {
        // ERR_NOSUCHCHANNEL (403)
        sendResponse(fd, ":ircserv 403 " + nick + " " + channelName + " :No such channel\r\n");
        return;
    }

    // Vérifier que l'opérateur est membre du canal
    if (!channel->isMember(fd))
    {
        // ERR_NOTONCHANNEL (442)
        sendResponse(fd, ":ircserv 442 " + nick + " " + channelName + " :You're not on that channel\r\n");
        return;
    }

    // Vérifier que l'opérateur a les privilèges d'opérateur
    if (!channel->isOperator(fd))
    {
        // ERR_CHANOPRIVSNEEDED (482)
        sendResponse(fd, ":ircserv 482 " + nick + " " + channelName + " :You're not channel operator\r\n");
        return;
    }

    // Trouver l'utilisateur cible
    int targetFd = -1;
    for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
    {
        if (it->second.getNickname() == targetNick)
        {
            targetFd = it->first;
            break;
        }
    }

    if (targetFd == -1)
    {
        // ERR_NOSUCHNICK (401)
        sendResponse(fd, ":ircserv 401 " + nick + " " + targetNick + " :No such nick/channel\r\n");
        return;
    }

    // Vérifier que l'utilisateur cible est dans le canal
    if (!channel->isMember(targetFd))
    {
        // ERR_USERNOTINCHANNEL (441)
        sendResponse(fd, ":ircserv 441 " + nick + " " + targetNick + " " + channelName + " :They aren't on that channel\r\n");
        return;
    }

    // Construire les informations de l'opérateur
    std::string user = client.getUsername();
    std::string host = client.getHostname();
    std::string fullMask = nick + "!" + user + "@" + host;

    // Construire le message KICK
    std::string kickMsg = ":" + fullMask + " KICK " + channelName + " " + targetNick + " :" + kickReason + "\r\n";

    // Envoyer le message KICK à TOUS les membres du canal (y compris celui qui se fait kick)
    std::vector<int> members = channel->getAllMembers();
    for (size_t i = 0; i < members.size(); i++)
    {
        sendResponse(members[i], kickMsg);
    }

    std::cout << "User " << targetNick << " kicked from " << channelName << " by " << nick << " (reason: " << kickReason << ")" << std::endl;

    // Retirer l'utilisateur du canal
    channel->removeMember(targetFd);
    
    // Mettre à jour la liste des canaux du client cible
    std::map<int, Client>::iterator targetClientIt = clients.find(targetFd);
    if (targetClientIt != clients.end())
    {
        targetClientIt->second.leaveChannel(channelName);
    }

    // Supprimer le canal s'il devient vide
    if (channel->isEmpty())
    {
        _server->removeChannel(channelName);
    }
}

void CommandHandler::handleTopic(int fd, const std::vector<std::string>& params)
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

    // Vérifier qu'il y a au moins un paramètre (nom du canal)
    if (params.empty())
    {
        // ERR_NEEDMOREPARAMS (461)
        sendResponse(fd, ":ircserv 461 " + client.getNickname() + " TOPIC :Not enough parameters\r\n");
        return;
    }

    std::string channelName = params[0];
    std::string nick = client.getNickname();

    // Vérifier que le canal existe
    Channel* channel = _server->getChannel(channelName);
    if (!channel)
    {
        // ERR_NOSUCHCHANNEL (403)
        sendResponse(fd, ":ircserv 403 " + nick + " " + channelName + " :No such channel\r\n");
        return;
    }

    // Vérifier que le client est membre du canal
    if (!channel->isMember(fd))
    {
        // ERR_NOTONCHANNEL (442)
        sendResponse(fd, ":ircserv 442 " + nick + " " + channelName + " :You're not on that channel\r\n");
        return;
    }

    // Si pas de deuxième paramètre, afficher le topic actuel
    if (params.size() == 1)
    {
        if (channel->getTopic().empty())
        {
            // RPL_NOTOPIC (331)
            sendResponse(fd, ":ircserv 331 " + nick + " " + channelName + " :No topic is set\r\n");
        }
        else
        {
            // RPL_TOPIC (332)
            sendResponse(fd, ":ircserv 332 " + nick + " " + channelName + " :" + channel->getTopic() + "\r\n");
        }
        return;
    }

    // Sinon, modifier le topic
    
    // Vérifier les permissions si le mode +t est activé
    if (channel->isTopicRestricted() && !channel->isOperator(fd))
    {
        // ERR_CHANOPRIVSNEEDED (482)
        sendResponse(fd, ":ircserv 482 " + nick + " " + channelName + " :You're not channel operator\r\n");
        return;
    }

    // Construire le nouveau topic
    std::string newTopic = params[1];
    
    // Si le topic commence par ':', l'enlever
    if (!newTopic.empty() && newTopic[0] == ':')
        newTopic = newTopic.substr(1);
    
    // Reconstituer le topic complet si il y a des espaces
    for (size_t i = 2; i < params.size(); i++)
        newTopic += " " + params[i];

    // Définir le nouveau topic
    channel->setTopic(newTopic);

    std::string user = client.getUsername();
    std::string host = client.getHostname();
    std::string fullMask = nick + "!" + user + "@" + host;

    // Notifier tous les membres du canal du changement de topic
    std::string topicMsg = ":" + fullMask + " TOPIC " + channelName + " :" + newTopic + "\r\n";
    std::vector<int> members = channel->getAllMembers();
    
    for (size_t i = 0; i < members.size(); i++)
    {
        sendResponse(members[i], topicMsg);
    }

    std::cout << "Topic changed in " << channelName << " by " << nick << ": " << newTopic << std::endl;
}

void CommandHandler::handleInvite(int fd, const std::vector<std::string>& params)
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

    // Vérifier qu'il y a au moins 2 paramètres (nickname et canal)
    if (params.size() < 2)
    {
        // ERR_NEEDMOREPARAMS (461)
        sendResponse(fd, ":ircserv 461 " + client.getNickname() + " INVITE :Not enough parameters\r\n");
        return;
    }

    std::string targetNick = params[0];
    std::string channelName = params[1];
    std::string nick = client.getNickname();

    // Vérifier que le canal existe
    Channel* channel = _server->getChannel(channelName);
    if (!channel)
    {
        // ERR_NOSUCHCHANNEL (403)
        sendResponse(fd, ":ircserv 403 " + nick + " " + channelName + " :No such channel\r\n");
        return;
    }

    // Vérifier que l'inviteur est membre du canal
    if (!channel->isMember(fd))
    {
        // ERR_NOTONCHANNEL (442)
        sendResponse(fd, ":ircserv 442 " + nick + " " + channelName + " :You're not on that channel\r\n");
        return;
    }

    // Vérifier que l'inviteur est opérateur du canal (nécessaire pour inviter)
    if (!channel->isOperator(fd))
    {
        // ERR_CHANOPRIVSNEEDED (482)
        sendResponse(fd, ":ircserv 482 " + nick + " " + channelName + " :You're not channel operator\r\n");
        return;
    }

    // Trouver l'utilisateur cible
    int targetFd = -1;
    Client* targetClient = NULL;
    
    for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
    {
        if (it->second.getNickname() == targetNick)
        {
            targetFd = it->first;
            targetClient = &(it->second);
            break;
        }
    }

    if (targetFd == -1 || !targetClient)
    {
        // ERR_NOSUCHNICK (401)
        sendResponse(fd, ":ircserv 401 " + nick + " " + targetNick + " :No such nick/channel\r\n");
        return;
    }

    // Vérifier que l'utilisateur cible n'est pas déjà dans le canal
    if (channel->isMember(targetFd))
    {
        // ERR_USERONCHANNEL (443)
        sendResponse(fd, ":ircserv 443 " + nick + " " + targetNick + " " + channelName + " :is already on channel\r\n");
        return;
    }

    // Ajouter l'utilisateur à la liste des invités
    channel->addInvited(targetFd);

    std::string user = client.getUsername();
    std::string host = client.getHostname();
    std::string fullMask = nick + "!" + user + "@" + host;

    // Envoyer la confirmation à l'inviteur
    // RPL_INVITING (341)
    sendResponse(fd, ":ircserv 341 " + nick + " " + targetNick + " " + channelName + "\r\n");

    // Envoyer l'invitation à l'utilisateur cible
    std::string inviteMsg = ":" + fullMask + " INVITE " + targetNick + " " + channelName + "\r\n";
    sendResponse(targetFd, inviteMsg);

    std::cout << "User " << targetNick << " invited to " << channelName << " by " << nick << std::endl;
}

void CommandHandler::handlePart(int fd, const std::vector<std::string>& params)
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

    // Vérifier qu'il y a au moins un paramètre (nom du canal)
    if (params.empty())
    {
        // ERR_NEEDMOREPARAMS (461)
        sendResponse(fd, ":ircserv 461 " + client.getNickname() + " PART :Not enough parameters\r\n");
        return;
    }

    // Parser les canaux (PART peut accepter plusieurs canaux séparés par des virgules)
    std::string channelList = params[0];
    std::vector<std::string> channels = split(channelList, ',');

    // Message de départ optionnel
    std::string partMessage = "";
    if (params.size() > 1)
    {
        partMessage = params[1];
        // Si le message commence par ':', l'enlever
        if (!partMessage.empty() && partMessage[0] == ':')
            partMessage = partMessage.substr(1);
        
        // Reconstituer le message complet si il y a des espaces
        for (size_t i = 2; i < params.size(); i++)
            partMessage += " " + params[i];
    }

    std::string nick = client.getNickname();
    std::string user = client.getUsername();
    std::string host = client.getHostname();
    std::string fullMask = nick + "!" + user + "@" + host;

    // Traiter chaque canal
    for (size_t i = 0; i < channels.size(); i++)
    {
        std::string channelName = channels[i];

        // Vérifier que le canal existe
        Channel* channel = _server->getChannel(channelName);
        if (!channel)
        {
            // ERR_NOSUCHCHANNEL (403)
            sendResponse(fd, ":ircserv 403 " + nick + " " + channelName + " :No such channel\r\n");
            continue;
        }

        // Vérifier que le client est membre du canal
        if (!channel->isMember(fd))
        {
            // ERR_NOTONCHANNEL (442)
            sendResponse(fd, ":ircserv 442 " + nick + " " + channelName + " :You're not on that channel\r\n");
            continue;
        }

        // Construire le message PART
        std::string partResponse = ":" + fullMask + " PART " + channelName;
        if (!partMessage.empty())
            partResponse += " :" + partMessage;
        partResponse += "\r\n";

        // Envoyer le message à TOUS les membres du canal (y compris celui qui part)
        std::vector<int> members = channel->getAllMembers();
        for (size_t j = 0; j < members.size(); j++)
        {
            sendResponse(members[j], partResponse);
        }

        std::cout << "Client " << nick << " left channel " << channelName;
        if (!partMessage.empty())
            std::cout << " (" << partMessage << ")";
        std::cout << std::endl;

        // Retirer le client du canal
        channel->removeMember(fd);
        
        // 🎯 LIGNE CRUCIALE AJOUTÉE :
        client.leaveChannel(channelName);

        // Supprimer le canal s'il devient vide
        if (channel->isEmpty())
        {
            _server->removeChannel(channelName);
        }
    }
}