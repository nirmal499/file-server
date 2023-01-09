#include <tcp_server/tcp_server.hpp>
#include <string.h>
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <unistd.h>
#include <poll.h>
#include <memory>
#include <sys/epoll.h>
#include <vector>
#include <thread_pool/pool.hpp>
#include <my_base/base.hpp>


namespace server
{
    using namespace std;

    // This function is available in this cpp file only.
    // Returns 0 if during any read inside this function returns 0 bytes
    int task(int m_client)
    {

        auto [bytes_read, my_str] = base_utility::read_String(m_client);
        if (bytes_read == 0)
        {
            return 0;
        }
        cout << bytes_read << "<->" << my_str.data() << "\n";
        string to_pass_my_str(reinterpret_cast<char *>(my_str.data()));

        // For testing the thread_pool
        // int i = 0;
        // while(i <= 10){
        //     sleep(2);
        //     ++i;
        // }

        base_utility::write_String(m_client, to_pass_my_str);

        return 999;
    }

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

    void myserver::start_for_echo()
    {
        cout << "ECHO sever is started single connection........: \n";
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
        cout << "File sever is started single connection........: \n";
        base_utility::CHECK(::listen(m_fd, MAX_CONN), "Error in listen");
        base_utility::CHECK(m_client = ::accept(m_fd, nullptr, nullptr), "Error in accept");

        int ret = handle_connection(m_client);
        if (ret == 0)
        {
            cout << "Returned 0...bytes_read is 0 becoz client must have closed connection\n";
        }
        else if (ret == -1)
        {
            cout << "Unsupported opertion...Close connection with client\n";
        }

        close(m_client);
    }

    void myserver::start_for_multi_threaded_file_server()
    {
        // We want two threads in out thread pool
        my_pool::thread_pool tp{2}; // It means there will be three instances of ./server becoz
        // 1 is main thread, and other 2 are the working thread

        cout << "Multi Threaded File ECHO server is started.......: \n";
        // we tell the API to use the default connection backlog, which is implementation-specific, by setting the backlog to 0
        base_utility::CHECK(::listen(m_fd, -1), "Error in listen");

        while (true)
        {
            base_utility::CHECK(m_client = ::accept(m_fd, nullptr, nullptr), "Error in accept");
            auto my_tuple = make_tuple(task, m_client);
            tp.do_task(my_tuple);
        }
    }

    void myserver::start_DONE_BY_SELECT()
    {
        cout << "Echo Server is started HANDLED BY SELECT.......: \n";

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

            int ret = select(fdMax + 1, &copy, nullptr, nullptr, nullptr);
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
                                FD_CLR(fd, &master);
                            }
                        }
                    }
                }
            }
        }
    }

    void myserver::start_DONE_BY_POLL()
    {
        cout << "Echo Server is started HANDLED BY POLL.......: \n";

        base_utility::CHECK(::listen(m_fd, MAX_CONN), "Error in listen");
        // base_utility::CHECK(m_client = ::accept(m_fd, nullptr, nullptr), "Error in accept");

        unique_ptr<struct pollfd[]> pollfds{};

        int maxFds = 0;

        pollfds = make_unique<struct pollfd[]>(FIRST_NUM_FDS);
        if (pollfds == nullptr)
        {
            throw runtime_error("Error in make_unique..\n");
        }

        int timeout = 30 * 1000; /** 30000 ms -> 30 sec*/
        maxFds = FIRST_NUM_FDS;

        pollfds[0] = {
            .fd = m_fd,
            .events = POLLIN,
            .revents = 0};
        int num_fds = 1;
        int ndfs;

        while (true)
        {
            // -1 indicates that our timeout is infinite. It means it will wait forever, until of the descriptor is ready to read
            ndfs = num_fds;
            base_utility::CHECK(poll(pollfds.get(), ndfs, -1), "Error in poll syscall.....");

            for (int i = 0; i < ndfs; ++i)
            {
                if (pollfds[i].fd <= 0)
                {
                    /**
                     * file descriptor = 0 is not expected, as these are socket fds not stdin
                     */
                    continue;
                }

                if ((pollfds[i].revents & POLLIN) == POLLIN)
                {
                    // fd is ready for reading

                    if (pollfds[i].fd == m_fd)
                    {
                        // request for new connection
                        base_utility::CHECK(m_client = ::accept(m_fd, nullptr, nullptr), "Error in accept");

                        // add new fd to pollfds
                        if (num_fds == maxFds)
                        {
                            // create space
                            // https://stackoverflow.com/questions/68210877/why-is-there-no-realloc-equivalent-to-the-new-delete-family

                            unique_ptr<struct pollfd[]> copy_pollfds = move(pollfds);
                            pollfds = make_unique<struct pollfd[]>(maxFds + FIRST_NUM_FDS);
                            if (pollfds == nullptr)
                            {
                                throw runtime_error("Error in make_unique....");
                            }

                            memcpy(pollfds.get(), copy_pollfds.get(), maxFds * sizeof(struct pollfd));
                            maxFds += FIRST_NUM_FDS;
                        }

                        /**
                         * Since I am incrementing num_fds here, I can't use it in for loop condition
                         */
                        num_fds++;
                        pollfds[num_fds - 1] = {
                            .fd = m_client,
                            .events = POLLIN,
                            .revents = 0};
                    }
                    else
                    {
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
        cout << "Echo Server is started HANDLED BY EPOLL.......: \n";

        base_utility::CHECK(::listen(m_fd, MAX_CONN), "Error in listen");
        // base_utility::CHECK(m_client = ::accept(m_fd, nullptr, nullptr), "Error in accept");

        int timeout = 30 * 1000; /** 30000 ms -> 30 sec*/

        int efd{};
        base_utility::CHECK(efd = epoll_create1(0), "Error in epoll_create...\n");
        struct epoll_event ev;
        ev.data.fd = m_fd;
        ev.events = EPOLLIN;

        array<struct epoll_event, MAX_CONN> ep_events;

        base_utility::CHECK(epoll_ctl(efd, EPOLL_CTL_ADD, m_fd, &ev), "Error in epoll_ctl ....\n");

        int no_fds = 0;

        while (true)
        {
            base_utility::CHECK(no_fds = epoll_wait(efd, ep_events.data(), MAX_CONN, -1), "Error in epoll_wait...\n");

            for (int i = 0; i < no_fds; ++i)
            {
                if ((ep_events[i].events & EPOLLIN) == EPOLLIN)
                {
                    if (ep_events[i].data.fd == m_fd)
                    {
                        // request for new connection
                        base_utility::CHECK(m_client = ::accept(m_fd, nullptr, nullptr), "Error in accept");

                        ev.data.fd = m_client;
                        ev.events = EPOLLIN;

                        base_utility::CHECK(epoll_ctl(efd, EPOLL_CTL_ADD, m_client, &ev), "Error in epoll_ctl ....\n");
                    }
                    else
                    {
                        // data from an existing client
                        if (int ret = task(ep_events[i].data.fd); ret == 0)
                        {
                            // returned 0 means fd closed the connection

                            // delete fd from epoll
                            base_utility::CHECK(epoll_ctl(efd, EPOLL_CTL_DEL, ep_events[i].data.fd, &ev), "Error in epoll_ctl...\n");

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

    int myserver::handle_connection(const int &fd)
    {
        // string resources_path = "/home/nbaskey/Desktop/cpp_projects/cpp_file_server/resources/client/";

        // 1@user0$user1;user2#provision_Certificate.pdf&user1;user2#pexels-alesia-kozik-6004828.jpg&

        auto [bytes_read, info_string] = base_utility::read_String(fd);
        if (bytes_read == 0)
        {
            return 0;
        }

        string to_pass_str(reinterpret_cast<char *>(info_string.data()));
        // cout << "str is : " << info_string.data() << '\n';

        if (to_pass_str[0] == '1')
        {
            /****Client wants to Upload files*****/
            /****Server needs to Download files****/

            auto [user_name_of_uploader, vec_for_FPTable, FPTable] = this->parse_for_uploading(to_pass_str);

            int unsuccessful_download_count = 0;
            size_t fileSize_read{};
            string final_path_to_file{};
            for (int i = 0; i < vec_for_FPTable.size(); ++i)
            {
                const string &file = vec_for_FPTable[i];

                final_path_to_file = SERVER_STORAGE_FOLDER + file;
                fileSize_read = base_utility::recvFile(fd, final_path_to_file);

                if (fileSize_read == 0)
                {
                    cout << "Some errored during receving/reading/downloading file " << file << " as the sent fileSize is 0\n";
                    unsuccessful_download_count++;
                }
                else
                {
                    base_utility::write_to_db(file, FPTable.at(file));
                    base_utility::write_String(fd, move(file));
                }
            }

            cout << "\nRESULTS: \n1. NO.OF DOWNLOADED FILES : " << vec_for_FPTable.size() - unsuccessful_download_count;
            cout << "\n2. NO.OF UNSUCCESSFUL DOWNLOADS : " << unsuccessful_download_count << "\n";

            return 1; // SUCCESS
        }
        else if (to_pass_str[0] == '2')
        {
            /****Client wants to Download files*****/
            /****Server needs to upload files******/

            auto [user_name_of_downloader, files_vec] = this->parse_for_downloading(to_pass_str);
            int unsuccessful_upload_count = 0;
            int error_occured_count = 0;
            size_t response_uint = 0;
            int allowed_or_not{};
            string final_path_to_file{};
            size_t fileSize_written{};
            for (const auto &s : files_vec)
            {

                allowed_or_not = base_utility::is_permitted(s, user_name_of_downloader);
                // cout << "allowed_or_not : " << allowed_or_not << "\n";
                if (allowed_or_not == -1)
                {
                    /**Some error occurred clients need to try again..*/
                    response_uint = 100;
                    error_occured_count++;
                }
                else if (allowed_or_not == 0)
                {
                    /**file 's' does not exists in the server */
                    response_uint = 200;
                    error_occured_count++;
                }
                else if (allowed_or_not == 1)
                {
                    /**user_name_of_downloader is not allowed to download the file 's'*/
                    response_uint = 300;
                    error_occured_count++;
                }
                else if (allowed_or_not == 2)
                {
                    /**user_name_of_downloader is allowed to download the file 's'*/
                    response_uint = 400;
                }
                // Server is writing the response to client to know the status of its request
                base_utility::write_UInt(fd, response_uint);
                if (allowed_or_not == 2)
                {
                    /**Server need to upload the file so that client can download*/
                    final_path_to_file = SERVER_STORAGE_FOLDER + s;
                    fileSize_written = base_utility::sendFile(fd, final_path_to_file);
                    if (fileSize_written == 0)
                    {
                        cout << "Some error occured during sending/writing/uploading file " << s << " as the sent fileSize is 0\n";
                        unsuccessful_upload_count++;
                    }
                }
            }

            cout << "\nRESULTS: \n1. NO.OF UPLOADED FILES : " << files_vec.size() - error_occured_count - unsuccessful_upload_count;
            cout << "\n2. NO.OF UNSUCCESSFUL UPLOADS : " << error_occured_count + unsuccessful_upload_count << "\n";

            return 1; // SUCCESS
        }
        else
        {
            cout << "Operation not supported....Let's close connection\n";
            return -1;
        }
    }

    tuple<string, vector<string>> myserver::parse_for_downloading(const string &str)
    {
        /* 2@user0$aaf53441-e1d2-41a3-a2c9-0287c3b068ea-os-dev.pdf&3eb2b3d9-4e5f-4c62-853d-d0ba62b8a3cb-sish.1.pdf */
        size_t pos_of_dollar = str.find_first_of('$');
        string part1 = str.substr(0, pos_of_dollar);

        string user_name_of_downloader = part1.substr(2);

        vector<string> works_vec = base_utility::split(str.substr(pos_of_dollar + 1), '&');

        return make_tuple(move(user_name_of_downloader), move(works_vec));
    }

    tuple<string, vector<string>, unordered_map<string, string>> myserver::parse_for_uploading(const string &str)
    {

        size_t pos_of_dollar = str.find_first_of('$');
        /* 1@user0$user1;user2#os-dev.pdf&user3;user4#sish.1.pdf */
        string part1 = str.substr(0, pos_of_dollar);
        // 2nd argument passed to substr is the no.of characters you want to extract

        string user_name_of_uploader = part1.substr(2);
        // cout << "user_name_of_uploader : " << user_name_of_uploader << "\n";

        // works_vec will contain : ['user1;user2#os-dev.pdf','user3;user4#sish.1.pdf']
        vector<string> works_vec = base_utility::split(str.substr(pos_of_dollar + 1), '&');
        for (auto &item : works_vec)
        {
            // cout << "Previous : " << item << "\n";
            item = user_name_of_uploader + ';' + item;
            // cout << "After : " << item << "\n\n";
        }
        // works_vec will contain : ['user0;user1;user2#os-dev.pdf','user0;user3;user4#sish.1.pdf']

        /* vector type should be the key_type of the unordered_map */
        // this vector is used to store the order of the elements sincee FPTable is unordered so the order is not retained
        vector<string> vec_used_as_a_helper_for_FPTable;

        // unordered_map<string,vector<string>> FPTable;
        unordered_map<string, string> FPTable;

        // vector<string> temp_permitted_users_vec;
        string temp;
        size_t pos_of_hash{};
        string permitted_users_str{};
        string my_uuid{};
        string new_file_name_with_uuid{};

        for (const auto &s : works_vec)
        {

            // s is 'user1;user2#os-dev.pdf'
            pos_of_hash = s.find_first_of('#');

            // temp_permitted_users_vec will contain: ['user1','user2']
            // temp_permitted_users_vec = base_utility::split(s.substr(0,pos_of_hash),';');
            permitted_users_str = s.substr(0, pos_of_hash);

            // temp contains: 'os-dev.pdf'
            temp = s.substr(pos_of_hash + 1);

            my_uuid = base_utility::generate_uuid();
            new_file_name_with_uuid = my_uuid + '-' + temp;
            // FPTable[new_file_name_with_uuid] = move(temp_permitted_users_vec);
            FPTable[new_file_name_with_uuid] = move(permitted_users_str);
            vec_used_as_a_helper_for_FPTable.push_back(move(new_file_name_with_uuid));
        }

        return make_tuple(move(user_name_of_uploader), move(vec_used_as_a_helper_for_FPTable), move(FPTable));
    }
}
