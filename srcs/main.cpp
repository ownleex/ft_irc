#include <Server.hpp>
#include <Client.hpp>

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

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "\nUsage: ./ircsrv <port> <password>\n\n";
        return 1;
    }
    
    int port;
    if (!isValidPortString(argv[1], port)) {
        std::cerr << "\nERROR: invalid port (must be a number between 1024-65535)\n\n";
        return 1;
    }
    
    std::string password = argv[2];
    if (password.empty()) {
        std::cerr << "\nERROR: password cannot be empty\n\n";
        return 1;
    }
    
    try {
        Server server(port, password);
        server.run_serv();
    }
    catch (const std::exception& e) {
        std::cerr << "\nERROR: " << e.what() << "\n\n";
        return 1;
    }
    
    return 0;
}
























/*

## connection complete commandes partielles

{
    printf "PASS "
    sleep 0.2
    printf "123\r\n"
    sleep 0.2
    printf "NI"
    sleep 0.2
    printf "CK "
    sleep 0.2
    printf "testuser\r\n"
    sleep 0.2
    printf "USER testuser 0 * :Test User\r\n"
    sleep 0.2
    cat
} | nc localhost 6666


## commande partielle

{
    printf "PA"
    cat
} | nc localhost 6666

## Commandes

# KICK
KICK user #channel [reason]

# INVITE
INVITE user #channel

# TOPIC
TOPIC #channel [new topic]

# MODE
    -i : MODE +i #channel (invite-only)
         MODE -i #channel (remove invite-only)

    -t : MODE +t #channel (topic restricted)
         MODE -t #channel (remove topic restricted)

    -k : MODE +k #channel key (set password)
         MODE -k #channel (remove password)

    -o : MODE +o user #channel (make user operator)
         MODE -o user #channel (remove operator status)

    -l : MODE +l limit #channel (set user limit)
         MODE -l #channel (remove user limit)

*/