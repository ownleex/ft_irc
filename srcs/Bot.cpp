#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstdlib>

bool isValidPortString(const char* str, int& port) {
    if (!str || *str == '\0') {
        return false;
    }
    
    char* endptr;
    long val = std::strtol(str, &endptr, 10);
    
    if (*endptr != '\0') {
        return false;
    }
    
    if (val < 1024 || val > 65535) {
        return false;
    }
    
    port = static_cast<int>(val);
    return true;
}

int main(int ac, char **av)
{
    if (ac != 3) {
        std::cerr << "\nUsage: ./channelbot <port> <password>\n\n";
        return 1;
    }
    
    int port;
    if (!isValidPortString(av[1], port)) {
        std::cerr << "\nERROR: invalid port (must be a number between 1024-65535)\n\n";
        return 1;
    }
    
    std::string password = av[2];
    if (password.empty()) {
        std::cerr << "\nERROR: password cannot be empty\n\n";
        return 1;
    }
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    if (connect(sock, (struct sockaddr *)& serv_addr, sizeof(serv_addr)) < 0)
    {
        std::cerr << "Connection failed" << std::endl;
        return 1;
    }

    std::string pass = std::string("PASS ") + password + "\r\n";
    std::string nick = "NICK bot\r\n";
    std::string user = "USER channelbot 0 * :Channel Bot\r\n";
    send(sock, pass.c_str(), pass.size(), 0);
    send(sock, nick.c_str(), nick.size(), 0);
    send(sock, user.c_str(), user.size(), 0);
    std::string channel = "#channelbot";
    std::string join = "JOIN " + channel + "\r\n";
    send(sock, join.c_str(), join.size(), 0);
    std::string topic = "TOPIC " + channel + " :Super channel du merveilleux BOT qui reste à votre disposition !!!\r\n";
    send(sock, topic.c_str(), topic.size(), 0);
    
    char buffer[512];
    while (true)
    {
        ssize_t bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);
        buffer[bytes] = '\0';
        std::string msg(buffer);
        std::cout << "[SERVER] " << msg;

        if (msg.find("PRIVMSG " + channel + " :BOT PING") != std::string::npos)
        {
            std::string pong = "PRIVMSG " + channel + " :PONG\r\n";
            send(sock, pong.c_str(), pong.size(), 0);
        }
        if (msg.find("PRIVMSG " + channel + " :BOT TIME") != std::string::npos)
        {
            time_t now = time(0);
            std::string time = "PRIVMSG " + channel + " :current server time: " + std::string(ctime(&now)) + "\r\n";
            send(sock, time.c_str(), time.size(), 0);
        }
    }
    close(sock);
    return (0);
}