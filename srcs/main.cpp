#include <Server.hpp>
#include <Client.hpp>

int main(int argc, char **argv)
{
    int port = std::atoi(argv[1]);
    std::string password = argv[2];

    if (argc == 3)
    {
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
    return (0);
}
