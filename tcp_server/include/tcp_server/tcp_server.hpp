#pragma once

#include<netinet/in.h>
#include<array>

#define PORT 8080
#define MAX_SIZE 256

namespace server
{
    using namespace std;

    class myserver
    {

        int m_fd{-1};
        int m_client{-1};

        struct sockaddr_in m_addr;

        int bind();

    public:
        explicit myserver(uint16_t port = PORT);
        ~myserver();
        void start();
    };

}