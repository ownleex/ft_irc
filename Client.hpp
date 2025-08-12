#ifndef CLIENT_HPP
#define CLIENT_HPP
#include <string>


class Client
{
    private:
        int _fd;
        std::string _username;

    public:
        Client(int fd);
        ~Client();

        int get_fd() const;
        void setUser(const std::string &user);
        const std::string &getUsername() const;
       
};

#endif