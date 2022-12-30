#include <iostream>
#include <tcp_client/tcp_client.hpp>
#include <unistd.h>

int main(int argc, char **argv)
{
    try
    {
        client::myclient client{PORT};
        client.echo();

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