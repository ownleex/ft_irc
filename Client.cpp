#include <Client.hpp>

Client::Client(int fd) : _fd(fd)
{

}

Client::~Client()
{

}

int Client::get_fd() const
{
    return(_fd);
}

void Client::setUser(const std::string &user)
{
    _username = user;
}

const std::string &Client::getUsername() const
{
    return (_username);
}