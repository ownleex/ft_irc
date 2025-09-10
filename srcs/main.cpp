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

# PASS
PASS [password]

# NICK
NICK new_nickname

# USER
USER new_nickname 0 * :Real Name

# JOIN
JOIN #channel [password]

# QUIT
QUIT :Goodbye!

# PART
PART #channel [reason]

# KICK
KICK user #channel [reason]

# INVITE
INVITE user #channel

# TOPIC
TOPIC #channel [new topic]

# MODE
    -i : MODE #channel +i (invite-only)
         MODE #channel -i (remove invite-only)

    -t : MODE #channel +t (topic restricted)
         MODE #channel -t (remove topic restricted)

    -k : MODE #channel +k key (set password)
         MODE #channel -k (remove password)

    -o : MODE #channel +o user (make user operator)
         MODE #channel -o user (remove operator status)

    -l : MODE #channel +l limit (set user limit)
         MODE #channel -l (remove user limit)

*/