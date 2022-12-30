#include <iostream>
#include <tcp_server/tcp_server.hpp>

int main()
{
    try
    {
        server::myserver server{PORT};
        server.start();

        return EXIT_SUCCESS;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Caught unhandled exception:\n";
        std::cerr << " - what(): " << e.what() << '\n';
    }
    catch (...)
    {
        std::cerr << "Caught unknown exception\n";
    }

    return EXIT_FAILURE;
}