#pragma once

#include <netinet/in.h>
#include <array>
#include <tuple>
#include <string>
#include <vector>

#define PORT 8080
#define MAX_SIZE 256

namespace client
{
    using namespace std;
    class myclient
    {
        int m_fd{-1};

        struct sockaddr_in m_addr;

        int connect();

    public:
        explicit myclient(uint16_t port = PORT);
        void echo();
        void start(const char *file);
        tuple<string, vector<string>> parse_for_both_uploading_and_downloading(const string &config_file);
        ~myclient();
    };

}