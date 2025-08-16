#pragma once

#include <string>
#include <cstdlib>
#include <set>
#include "Server.hpp"

class Client
{
    private:
        int _fd;
        std::string _username;
        std::string _nickname;
        std::string _realname;
        std::string _hostname;
        std::string _buffer;
        bool _authenticated;
        bool _registered;
        std::set<std::string> _channels;

    public:
        Client(int fd);
        ~Client();

        // Getters
        int get_fd() const;
        const std::string &getUsername() const;
        const std::string &getNickname() const;
        const std::string &getRealname() const;
        const std::string &getHostname() const;
        const std::string &getBuffer() const;
        bool isAuthenticated() const;
        bool isRegistered() const;

        // Setters
        void setUser(const std::string &user);
        void setNickname(const std::string &nickname);
        void setRealname(const std::string &realname);
        void setHostname(const std::string &hostname);
        void setBuffer(const std::string &buffer);
        void setAuthenticated(bool authenticated);
        void setRegistered(bool registered);

};
