#pragma once

#include<netinet/in.h>
#include<array>
#include<unordered_map>
#include<string>
#include<vector>

#define PORT 8080
#define MAX_SIZE 256
#define MAX_CONN 4
#define FIRST_NUM_FDS 2

namespace server
{
    using namespace std;

    class myserver
    {

        int m_fd{-1};
        int m_client{-1};

        struct sockaddr_in m_addr;

        int bind();
        int task(const int &m_client);
        tuple<string, vector<string>, unordered_map<string, vector<string>>> parse(const string &str);
        vector<string> split(const string &str, char delim);
        int handle_connection(const int &fd);

    public:
        explicit myserver(uint16_t port = PORT);
        ~myserver();
        void start_for_echo();
        void start_for_file_server();
        void start_DONE_BY_SELECT();
        void start_DONE_BY_POLL();
        void start_DONE_BY_EPOLL();

    };

}