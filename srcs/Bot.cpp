#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

int main()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(6667);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    if (connect(sock, (struct sockaddr *)& serv_addr, sizeof(serv_addr)) < 0)
    {
        std::cerr << "Connection failed" << std::endl;
        return 1;
    }

    std::string pass = "PASS 123\r\n";
    std::string nick = "NICK bot\r\n";
    std::string user = "USER channelbot 0 * :Channel Bot\r\n";
    send(sock, pass.c_str(), pass.size(), 0);
    send(sock, nick.c_str(), nick.size(), 0);
    send(sock, user.c_str(), user.size(), 0);
    std::string channel = "#General";
    std::string join = "JOIN " + channel + "\r\n";
    send(sock, join.c_str(), join.size(), 0);

    char buffer[512];
    while (true)
    {
        ssize_t bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);
        buffer[bytes] = '\0';
        std::string msg(buffer);
        std::cout << "[SERVER] " << msg;

        if (msg.find("PING") == 0)
        {
            std::string pong = "PONG " + msg.substr(5);
            send(sock, pong.c_str(), pong.size(), 0);
        }
        if (msg.find("PRIVMSG " + channel + " :TIME") != std::string::npos)
        {
            time_t now = time(0);
            std::string time = "PRIVMSG " + channel + " :current server time: " + std::string(ctime(&now)) + "\r\n";
            send(sock, time.c_str(), time.size(), 0);
        }
    }
    close(sock);
    return (0);
}