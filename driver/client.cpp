#include <iostream>
#include <tcp_client/tcp_client.hpp>
#include <unistd.h>

int main(int argc, char **argv)
{
    try
    {   
        if (argc < 2)
        {
            std::cout << "Please provide the config file!!!!\n";
            return EXIT_FAILURE;
        }

        client::myclient client{PORT};

    #if SERVER_TYPE == 1
            client.echo();
    #elif SERVER_TYPE == 2
            client.start(argv[1]);
    #elif SERVER_TYPE == 3
            client.echo_for_multithreaded_server();
    #elif SERVER_TYPE == 4
            client.echo();
    #elif SERVER_TYPE == 5
            client.echo();
    #elif SERVER_TYPE == 6
            client.echo();
    #endif

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