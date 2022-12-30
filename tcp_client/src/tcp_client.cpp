#include <tcp_client/tcp_client.hpp>
#include <string.h>
#include <my_base/base.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <fstream>

namespace client
{
    using namespace std;

    myclient::myclient(uint16_t port)
    {
        base_utility::CHECK(m_fd = socket(AF_INET, SOCK_STREAM, 0), "Error in socket()");

        m_addr.sin_family = AF_INET;
        m_addr.sin_port = htons(port);
        m_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

        base_utility::CHECK(this->connect(), "Error in connect()");
    }

    int myclient::connect()
    {
        return ::connect(m_fd, reinterpret_cast<struct sockaddr *>(&m_addr), sizeof(m_addr));
    }

    myclient::~myclient()
    {
        close(m_fd);
    }

    void myclient::echo(){
        while(true){
            string sendbuf{};

            // cin >> sendbuf;
            getline(cin,sendbuf);

            base_utility::write_String(m_fd,sendbuf);
            auto [_,my_str] = base_utility::read_String(m_fd,sendbuf.size());
            
            cout << my_str.data() << "\n";
        }
    }

}