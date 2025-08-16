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
        std::cerr << "\nUsage: " << argv[0] << " <port> <password>\n\n";
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
