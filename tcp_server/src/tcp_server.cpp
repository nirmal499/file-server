#include <tcp_server/tcp_server.hpp>
#include <string.h>
#include <my_base/base.hpp>
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <unistd.h>
#include <sstream>
#include <poll.h>
#include <memory>
#include <sys/epoll.h>
#include <vector>

namespace server
{
    using namespace std;

    myserver::myserver(uint16_t port)
    {
        base_utility::CHECK(m_fd = socket(AF_INET, SOCK_STREAM, 0), "Error in socket()");

        m_addr.sin_family = AF_INET;
        m_addr.sin_port = htons(port);
        m_addr.sin_addr.s_addr = htonl(INADDR_ANY);

        base_utility::CHECK(this->bind(), "Error in bind()");
    }

    int myserver::bind()
    {
        return ::bind(m_fd, reinterpret_cast<struct sockaddr *>(&m_addr), sizeof(m_addr));
    }

    // Returns 0 if during any read inside this function returns 0 bytes
    int myserver::task(const int &m_client)
    {

        auto [bytes_read,my_str] = base_utility::read_String(m_client);
        if (bytes_read == 0){
            return 0;
        }
        cout << bytes_read << "<->" << my_str.data() << "\n";
        string to_pass_my_str(reinterpret_cast<char *>(my_str.data()));
        base_utility::write_String(m_client,to_pass_my_str);

        return 999;
    }

    void myserver::start_for_echo()
    {
        base_utility::CHECK(::listen(m_fd, MAX_CONN), "Error in listen");
        base_utility::CHECK(m_client = ::accept(m_fd, nullptr, nullptr), "Error in accept");

        while (true)
        {
            if (int ret = task(m_client); ret == 0)
            {
                break;
            }
        }

        close(m_client);
    }

    void myserver::start_for_file_server()
    {
        base_utility::CHECK(::listen(m_fd, MAX_CONN), "Error in listen");
        base_utility::CHECK(m_client = ::accept(m_fd, nullptr, nullptr), "Error in accept");

        if (int ret = handle_connection(m_client); ret == 0)
        {   
            cout << "Returned 0...\n";
            return;
        }

        close(m_client);
    }

    void myserver::start_DONE_BY_SELECT()
    {
        base_utility::CHECK(::listen(m_fd, MAX_CONN), "Error in listen");

        fd_set master;
        FD_ZERO(&master);
        FD_SET(m_fd, &master);

        int fdMax = m_fd;

        struct timeval tv = {
            .tv_sec = 30,
            .tv_usec = 0};

        while (true)
        {
            fd_set copy = master;

            int ret = select(fdMax + 1, &copy, nullptr, nullptr, &tv);
            base_utility::CHECK(ret, "Error in select syscall..");

            if (ret == 0)
            {
                cout << "Waiting no client tried to connect ...\n";
            }
            else
            {
                for (int fd = 0; fd < (fdMax + 1); ++fd)
                {
                    if (FD_ISSET(fd, &copy))
                    {
                        if (fd == m_fd)
                        {
                            // request for new connection

                            base_utility::CHECK(m_client = ::accept(m_fd, nullptr, nullptr), "Error in accept");
                            FD_SET(m_client, &master);

                            if (m_client > fdMax)
                            {
                                fdMax = m_client;
                            }
                        }
                        else
                        {
                            // data from the connections, receive it

                            if (int ret = task(fd); ret == 0)
                            {
                                // returned 0 means fd closed the connection
                                close(fd);
                                FD_CLR(fd,&master);
                            }
                        }
                    }
                }
            }
        }
    }

    void myserver::start_DONE_BY_POLL()
    {
        base_utility::CHECK(::listen(m_fd, MAX_CONN), "Error in listen");
        // base_utility::CHECK(m_client = ::accept(m_fd, nullptr, nullptr), "Error in accept");

        unique_ptr<struct pollfd[]> pollfds{};
        
        int maxFds = 0;
        
        pollfds = make_unique<struct pollfd[]>(FIRST_NUM_FDS);
        if(pollfds == nullptr){
            throw runtime_error("Error in make_unique..\n");
        }

        int timeout = 30 * 1000; /** 30000 ms -> 30 sec*/
        maxFds = FIRST_NUM_FDS;

        pollfds[0] = {
            .fd = m_fd,
            .events = POLLIN,
            .revents = 0
        };
        int num_fds = 1;
        int ndfs;

        while(true){
            // -1 indicates that our timeout is infinite
            ndfs = num_fds;
            base_utility::CHECK(poll(pollfds.get(),ndfs,-1),"Error in poll syscall.....");

            for(int i=0 ; i<ndfs ;++i){
                if(pollfds[i].fd <= 0){
                    /**
                     * file descriptor = 0 is not expected, as these are socket fds not stdin
                    */
                    continue;
                }

                if((pollfds[i].revents & POLLIN) == POLLIN){
                    // fd is ready for reading

                    if(pollfds[i].fd == m_fd){
                        // request for new connection
                        base_utility::CHECK(m_client = ::accept(m_fd, nullptr, nullptr), "Error in accept");

                        // add new fd to pollfds
                        if(num_fds == maxFds){
                            // create space
                            // https://stackoverflow.com/questions/68210877/why-is-there-no-realloc-equivalent-to-the-new-delete-family

                            unique_ptr<struct pollfd[]> copy_pollfds = move(pollfds);
                            pollfds = make_unique<struct pollfd[]>(maxFds + FIRST_NUM_FDS);
                            if(pollfds == nullptr){
                                throw runtime_error("Error in make_unique....");
                            }

                            memcpy(pollfds.get(),copy_pollfds.get(),maxFds * sizeof(struct pollfd));
                            maxFds += FIRST_NUM_FDS;

                        }

                        /**
                         * Since I am incrementing num_fds here, I can't use it in for loop condition
                        */
                        num_fds++;
                        pollfds[num_fds-1] = {
                            .fd = m_client,
                            .events = POLLIN,
                            .revents = 0
                        };
                    }else{
                        // data from existing connection, recieve it
                        // cout << "fd is : " << pollfds[i].fd << "\n";
                        if (int ret = task(pollfds[i].fd); ret == 0)
                        {
                            // returned 0 means fd closed the connection
                            close(pollfds[i].fd);
                            pollfds[i].fd *= -1; /* make it negative so it is ignored in the future*/
                        }
                    }
                }
            }
        }
        
    }

    void myserver::start_DONE_BY_EPOLL()
    {
        base_utility::CHECK(::listen(m_fd, MAX_CONN), "Error in listen");
        // base_utility::CHECK(m_client = ::accept(m_fd, nullptr, nullptr), "Error in accept");

        int timeout = 30 * 1000; /** 30000 ms -> 30 sec*/

        int efd{};
        base_utility::CHECK(efd = epoll_create1(0),"Error in epoll_create...\n");
        struct epoll_event ev;
        ev.data.fd = m_fd;
        ev.events = EPOLLIN;

        array<struct epoll_event,MAX_CONN> ep_events;

        base_utility::CHECK(epoll_ctl(efd,EPOLL_CTL_ADD,m_fd,&ev),"Error in epoll_ctl ....\n");

        int no_fds = 0;
        while (true)
        {
            base_utility::CHECK(no_fds =  epoll_wait(efd,ep_events.data(),MAX_CONN,-1),"Error in epoll_wait...\n");

            for(int i = 0; i < no_fds ;++i){
                if((ep_events[i].events & EPOLLIN) == EPOLLIN){
                    if(ep_events[i].data.fd == m_fd){
                        // request for new connection
                        base_utility::CHECK(m_client = ::accept(m_fd, nullptr, nullptr), "Error in accept");

                        ev.data.fd = m_client;
                        ev.events = EPOLLIN;

                        base_utility::CHECK(epoll_ctl(efd,EPOLL_CTL_ADD,m_client,&ev),"Error in epoll_ctl ....\n");
                    }else{
                        // data from an existing client
                        if (int ret = task(ep_events[i].data.fd); ret == 0)
                        {
                            // returned 0 means fd closed the connection

                            // delete fd from epoll
                            base_utility::CHECK(epoll_ctl(efd,EPOLL_CTL_DEL,ep_events[i].data.fd,&ev),"Error in epoll_ctl...\n");

                            close(ep_events[i].data.fd);

                        }
                    }
                }
            }
        }
    }

    myserver::~myserver()
    {
        close(m_fd);
    }

    int myserver::handle_connection(const int &fd){
        string resources_path = "/home/nbaskey/Desktop/cpp_projects/cpp_file_server/resources/client/";

        auto [bytes_read,info_string] = base_utility::read_String(fd);
        if(bytes_read == 0){
            return 0;
        }

        string to_pass_str(reinterpret_cast<char *>(info_string.data()));
        cout << "str is : " << info_string.data() << '\n';
        auto [part1,vec_for_FPTable,FPTable] = this->parse(to_pass_str);

        int unsuccessful_download_count = 0;
        for(int i = 0; i < vec_for_FPTable.size() ; ++i){
            const string &file = vec_for_FPTable[i];

            size_t fileSize_read = base_utility::recvFile(fd,resources_path+file);

            if(fileSize_read == 0){
                cout << "Some errored during receving/reading/downloading file " << file << " as the sent fileSize is 0\n";
                unsuccessful_download_count++;
            }else{
                base_utility::write_String(fd,file);
            }

        }

        cout << "\nRESULTS: \n1. NO.OF DOWNLOADED FILES : " << vec_for_FPTable.size() - unsuccessful_download_count;
        cout << "\n2. NO.OF UNSUCCESSFUL DOWNLOADS : " << unsuccessful_download_count << "\n";

        return 1;

    }

    tuple<string, vector<string>, unordered_map<string, vector<string>>> myserver::parse(const string &str){

        size_t pos_of_dollar = str.find_first_of('$');
        /* 1@user0$user1;user2#os-dev.pdf&user3;user4#sish.1.pdf */
        string part1 = str.substr(0,pos_of_dollar);
        string part2 = str.substr(pos_of_dollar + 1);

        // works_vec will contain : ['user1;user2#os-dev.pdf','user3;user4#sish.1.pdf']
        vector<string> works_vec = this->split(part2, '&');

        /* vector type should be the key_type of the unordered_map */
        vector<string> vec_used_as_a_helper_for_FPTable;

        unordered_map<string,vector<string>> FPTable;

        vector<string> temp_permitted_users_vec;
        string temp;

        for(const auto &s: works_vec){
            
            // s is 'user1;user2#os-dev.pdf'
            size_t pos_of_hash = s.find_first_of('#');

            // temp_permitted_users_vec will contain: ['user1','user2']
            temp_permitted_users_vec = this->split(s.substr(0,pos_of_hash),';');
            
            // temp contains: 'os-dev.pdf'
            temp = s.substr(pos_of_hash + 1);

            string my_uuid = base_utility::generate_uuid();
            string new_file_name_with_uuid = my_uuid + '-' + temp;
            FPTable[new_file_name_with_uuid] = move(temp_permitted_users_vec);
            vec_used_as_a_helper_for_FPTable.push_back(move(new_file_name_with_uuid));
        }

        return make_tuple(move(part1),move(vec_used_as_a_helper_for_FPTable),move(FPTable));
    }

    vector<string> myserver::split(const string &str, char delim)
    {
        vector<string> result;
        stringstream ss(str);
        string item;

        while (getline(ss, item, delim))
        {
            result.push_back(item);
        }

        return result;
    }


}