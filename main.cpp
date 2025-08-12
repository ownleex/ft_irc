#include <Server.hpp>
#include <Client.hpp>

int main(int port, std::string password)
{
    try
    {
        Server server(0000, "1234");
        server.run_serv();
    }
    catch(const std::exception& e)
    {
        std::cerr << "ERROR" << e.what() << '\n';
        return (1);
    }
    return (0);
}