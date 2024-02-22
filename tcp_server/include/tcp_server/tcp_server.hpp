#pragma once

#include <netinet/in.h>
#include <array>
#include <unordered_map>
#include <string>
#include <vector>

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
        tuple<string, vector<string>, unordered_map<string, string>> parse_for_uploading(const string &str);
        tuple<string, vector<string>> parse_for_downloading(const string &str);
        int handle_connection(const int &fd);

    public:
        explicit myserver(uint16_t port = PORT);
        ~myserver();
        void start_for_echo();
        void start_for_file_server();
        void start_DONE_BY_SELECT();
        void start_DONE_BY_POLL();
        void start_DONE_BY_EPOLL();
        void start_for_multi_threaded_echo_server();
        // void start_for_multi_threaded_file_server();
        void start_file_server_DONE_BY_SELECT();
        void start_file_server_DONE_BY_POLL();
        void start_file_server_DONE_BY_EPOLL();
    };


}