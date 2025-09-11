#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <csignal>
#include <cerrno>

volatile sig_atomic_t g_stop = 0;
int g_sock = -1;

void handleSignal(int){ 
    g_stop = 1;
    close(g_sock);
}

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
    std::signal(SIGINT, handleSignal);

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
    
    g_sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    if (connect(g_sock, (struct sockaddr *)& serv_addr, sizeof(serv_addr)) < 0)
    {
        std::cerr << "Connection failed" << std::endl;
        return 1;
    }

    std::string pass = std::string("PASS ") + password + "\r\n";
    std::string nick = "NICK bot\r\n";
    std::string user = "USER channelbot 0 * :Channel Bot\r\n";
    send(g_sock, pass.c_str(), pass.size(), 0);
    send(g_sock, nick.c_str(), nick.size(), 0);
    send(g_sock, user.c_str(), user.size(), 0);
    std::string channel = "#channelbot";
    std::string join = "JOIN " + channel + "\r\n";
    send(g_sock, join.c_str(), join.size(), 0);
    std::string topic = "TOPIC " + channel + " :Super channel du merveilleux BOT qui reste à votre disposition !!!\r\n";
    send(g_sock, topic.c_str(), topic.size(), 0);
    
    char buffer[512];
    while (!g_stop)
    {
        ssize_t bytes = recv(g_sock, buffer, sizeof(buffer) - 1, 0);
        buffer[bytes] = '\0';
        std::string msg(buffer);
        std::cout << "[SERVER] " << msg;
        std::string replyTo = "";
        
        if (msg.find("PRIVMSG " + channel + " :") != std::string::npos)
        {
            // si dans le channel -> répondre dans le channel
            replyTo = channel;
        }
        else if (msg.find("PRIVMSG ") != std::string::npos)
        {
            // Sinon c' est unmessage privé -> répondre à l'expéditeur
            size_t exclamPos = msg.find('!');
            if (exclamPos != std::string::npos)
                replyTo = msg.substr(1, exclamPos - 1);
        }
        

        if (msg.find(" :PING") != std::string::npos)
        {
            std::string response = "PRIVMSG " + replyTo + " :PONG\r\n";
            send(g_sock, response.c_str(), response.size(), 0);
        }
        else if (msg.find(" :TIME") != std::string::npos)
        {
            time_t now = time(0);
            std::string time_str = std::string(ctime(&now));
            if (!time_str.empty() && time_str[time_str.length()-1] == '\n') {
                time_str.erase(time_str.length()-1);
            }
            std::string response = "PRIVMSG " + replyTo + " :current server time: " + time_str + "\r\n";
            send(g_sock, response.c_str(), response.size(), 0);
        }
    }

    close(g_sock);
    return (0);
}
