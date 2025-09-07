#include <Client.hpp>

Client::Client(int fd) : _fd(fd), _authenticated(false), _registered(false)
{

}

Client::~Client()
{

}

// Getters

int Client::get_fd() const
{
    return(_fd);
}

const std::string &Client::getUsername() const
{
    return (_username);
}

const std::string &Client::getNickname() const
{
    return (_nickname);
}

const std::string &Client::getRealname() const
{
    return (_realname);
}

const std::string &Client::getHostname() const
{
    return (_hostname);
}

std::string &Client::getBuffer()
{
    return (_buffer);
}

bool Client::isAuthenticated() const
{
    return (_authenticated);
}

bool Client::isRegistered() const
{
    return (_registered);
}

// Setters

void Client::setUser(const std::string &user)
{
    _username = user;
}

void Client::setNickname(const std::string &nickname)
{
    _nickname = nickname;
}

void Client::setRealname(const std::string &realname)
{
    _realname = realname;
}

void Client::setHostname(const std::string &hostname)
{
    _hostname = hostname;
}

void Client::setBuffer(const std::string &buffer)
{
    _buffer = buffer;
}

void Client::setAuthenticated(bool authenticated)
{
    _authenticated = authenticated;
}

void Client::setRegistered(bool registered)
{
    _registered = registered;
}

void Client::appendToBuffer(const std::string &data)
{
    _buffer += data;
}

void Client::joinChannel(const std::string& channelName)
{
    _channels.insert(channelName);
}

void Client::leaveChannel(const std::string& channelName)
{
    _channels.erase(channelName);
}

bool Client::isInChannel(const std::string& channelName) const
{
    return _channels.find(channelName) != _channels.end();
}

const std::set<std::string>& Client::getChannels() const
{
    return _channels;
}