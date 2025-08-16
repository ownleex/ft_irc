#include <Server.hpp>
#include <Client.hpp>

int main(int argc, char **argv)
{
    if (argc == 3)
    {
        int port = std::atoi(argv[1]);
        std::string password = argv[2];

        try
        {
            Server server(port, password);
            server.run_serv();
        }
        catch(const std::exception& e)
        {
            std::cerr << "ERROR" << e.what() << std::endl;
            return (1);
        }
    }
    else
        std::cout << "\nusage : ./ircsrv port password\n\n";

    return (0);
}
