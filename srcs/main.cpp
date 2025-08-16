#include <Server.hpp>
#include <Client.hpp>

int main(int argc, char **argv)
{
    if (argc == 3)
    {
        int port = std::atoi(argv[1]);
        if (port < 1024 | port > 65535)
        {
            std::cerr << "\nERROR: invalid port (1024-65535)\n\n";
            std::exit(1);
        }

        std::string password = argv[2];

        try
        {
            Server server(port, password);
            server.run_serv();
        }
        catch(const std::exception& e)
        {
            std::cerr << "ERROR" << e.what() << std::endl;
            std::exit(1);
        }
    }
    else
    {
        std::cerr << "\nusage : ./ircsrv port password\n\n";
        std::exit(1);
    }
    return (0);
}
