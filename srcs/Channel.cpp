#include "Channel.hpp"

Channel::Channel(const std::string &name) 
    : _name(name), _topic(""), _password(""), _inviteOnly(false), 
      _topicRestricted(true), _hasPassword(false), _userLimit(0)
{
}

Channel::~Channel()
{
}

// Getters
const std::string &Channel::getName() const
{
    return _name;
}

const std::string &Channel::getTopic() const
{
    return _topic;
}

const std::string &Channel::getPassword() const
{
    return _password;
}

const std::set<int> &Channel::getMembers() const
{
    return _members;
}

const std::set<int> &Channel::getOperators() const
{
    return _operators;
}

bool Channel::isInviteOnly() const
{
    return _inviteOnly;
}

bool Channel::isTopicRestricted() const
{
    return _topicRestricted;
}

bool Channel::hasPassword() const
{
    return _hasPassword;
}

int Channel::getUserLimit() const
{
    return _userLimit;
}

size_t Channel::getMemberCount() const
{
    return _members.size();
}

// Setters
void Channel::setTopic(const std::string &topic)
{
    _topic = topic;
}

void Channel::setPassword(const std::string &password)
{
    _password = password;
    _hasPassword = !password.empty();
}

void Channel::setInviteOnly(bool inviteOnly)
{
    _inviteOnly = inviteOnly;
}

void Channel::setTopicRestricted(bool topicRestricted)
{
    _topicRestricted = topicRestricted;
}

void Channel::setUserLimit(int limit)
{
    _userLimit = limit;
}

// Gestion des membres
bool Channel::addMember(int clientFd)
{
    if (_userLimit > 0 && _members.size() >= static_cast<size_t>(_userLimit))
        return false;
    
    _members.insert(clientFd);
    
    // Si c'est le premier membre, il devient automatiquement opérateur
    if (_members.size() == 1)
        _operators.insert(clientFd);
    
    return true;
}

bool Channel::removeMember(int clientFd)
{
    _members.erase(clientFd);
    _operators.erase(clientFd);
    _invited.erase(clientFd);
    return true;
}

bool Channel::isMember(int clientFd) const
{
    return _members.find(clientFd) != _members.end();
}

// Gestion des opérateurs
bool Channel::addOperator(int clientFd)
{
    if (!isMember(clientFd))
        return false;
    
    _operators.insert(clientFd);
    return true;
}

bool Channel::removeOperator(int clientFd)
{
    _operators.erase(clientFd);
    return true;
}

bool Channel::isOperator(int clientFd) const
{
    return _operators.find(clientFd) != _operators.end();
}

// Gestion des invitations
bool Channel::addInvited(int clientFd)
{
    _invited.insert(clientFd);
    return true;
}

bool Channel::removeInvited(int clientFd)
{
    _invited.erase(clientFd);
    return true;
}

bool Channel::isInvited(int clientFd) const
{
    return _invited.find(clientFd) != _invited.end();
}

// Vérifications
bool Channel::canJoin(int clientFd, const std::string &password) const
{
    // Vérifier la limite d'utilisateurs
    if (_userLimit > 0 && _members.size() >= static_cast<size_t>(_userLimit))
        return false;
    
    // Vérifier le mode invite-only
    if (_inviteOnly && !isInvited(clientFd))
        return false;
    
    // Vérifier le mot de passe
    if (_hasPassword && password != _password)
        return false;
    
    return true;
}

bool Channel::isEmpty() const
{
    return _members.empty();
}

// Utilitaires
std::string Channel::getModeString() const
{
    std::string modes = "+";
    
    if (_inviteOnly)
        modes += "i";
    if (_topicRestricted)
        modes += "t";
    if (_hasPassword)
        modes += "k";
    if (_userLimit > 0)
        modes += "l";
    
    return modes;
}

std::vector<int> Channel::getAllMembers() const
{
    std::vector<int> members;
    for (std::set<int>::const_iterator it = _members.begin(); it != _members.end(); ++it)
    {
        members.push_back(*it);
    }
    return members;
}