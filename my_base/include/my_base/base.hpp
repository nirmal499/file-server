#pragma once

#include <iostream> /* For the `uint32_t` in this file and `runtime_error` in the base.cpp  */
#include <unistd.h> /* For the `close()` in the tcp_server.cpp */
#include <tuple>
#include <string.h>
#include <variant>

/* MY_BUFF_SIZE is 4*1024 indicating 4KB used in recvBuffer and sendBuffer */
/* MY_FILE_BUFF_SIZE is 64*1024 indicating 64KB used in sendFile and recvFile*/
/* MY_CHAR_BUFF_SIZE is 256 indicating 256B used in writeString*/

#define MY_BUFF_SIZE 65536
#define STR_MAX_SIZE 500

namespace base_utility
{

    /* inline must be defined at prototyping */
    using namespace std;
    /* Checks for -1 as error. If error_int is -1 then it throws runtime_error */
    static inline void CHECK(const int &error_int, const string &str)
    {
        if (error_int == -1)
        {
            throw runtime_error(strerror(errno));
            // throw runtime_error(str);

        }
    }

    void my_write(const int &fd,const uint8_t buffer[],const size_t &bufferSize);
    size_t my_read(const int &fd,uint8_t buffer[],const size_t &bufferSize);

    void write_UInt(const int &fd,size_t &data);
    void write_String(const int &fd,const string &buf);

    tuple<size_t,size_t> read_UInt(const int &fd);
    tuple<size_t,array<uint8_t,STR_MAX_SIZE>> read_String(const int &fd);

    size_t sendFile(const int &fd, const string &fileName);
    size_t recvFile(const int &fd, const string &fileName);

}