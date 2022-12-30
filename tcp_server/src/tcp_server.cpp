#include <tcp_server/tcp_server.hpp>
#include <string.h>
#include <my_base/base.hpp>
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <unistd.h>
#include <sstream>

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

    void myserver::start()
    {
        base_utility::CHECK(::listen(m_fd, 0), "Error in listen");
        base_utility::CHECK(m_client = ::accept(m_fd, nullptr, nullptr), "Error in accept");

        while(true){
            
            auto [bytes_read,length] = base_utility::read_UInt(m_client);
            if(bytes_read != 0){
                auto [bytes_read,my_str] = base_utility::read_String(m_client,length);
                if(bytes_read != 0){
                    std::cout << length << "<->" << my_str.data() << "\n";
                    base_utility::my_write(m_client,my_str.data(),length);
                }else{
                    break;
                }
            }else{
                break;
            }
        }

        close(m_client);

        // while (true)
        // {
        //     cout << "Waiting for connections....\n";
        //     /* We are passing nullptr cause we don't want client information */
        //     base_utility::CHECK(m_client = ::accept(m_fd, nullptr, nullptr), "Error in accept");
        //     cout << "Connected!\n";

        //     handle_connection();
        //     break;
        // }
    }

    myserver::~myserver()
    {
        close(m_fd);
    }

}