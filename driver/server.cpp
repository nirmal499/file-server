#include <iostream>
#include <tcp_server/tcp_server.hpp>

int main()
{
    try
    {
        server::myserver server{PORT};
        // server.start_for_echo();
        server.start_for_file_server();
        // server.start_DONE_BY_SELECT();
        // server.start_DONE_BY_POLL();
        // server.start_DONE_BY_EPOLL();

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