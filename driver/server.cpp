#include <iostream>
#include <tcp_server/tcp_server.hpp>

int main()
{
    try
    {
        server::myserver server{PORT};

    #if SERVER_TYPE == 1
            server.start_for_echo();
    #elif SERVER_TYPE == 2
            server.start_for_file_server();
    #elif SERVER_TYPE == 3
            server.start_for_multi_threaded_file_server();
    #elif SERVER_TYPE == 4
            server.start_DONE_BY_SELECT();
    #elif SERVER_TYPE == 5
            server.start_DONE_BY_POLL();
    #elif SERVER_TYPE == 6
            server.start_DONE_BY_EPOLL();
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