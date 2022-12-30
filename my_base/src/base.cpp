#include "../include/my_base/base.hpp"
#include <algorithm>

/**
 *  Writes data to fd from buffer until bufferSize value is met
*/
void base_utility::my_write(const int &fd,const uint8_t buffer[],const size_t &bufferSize){
    size_t total_bytes_written = 0;

    while (total_bytes_written != bufferSize)
    {
        ssize_t bytes_written = ::write(fd, &buffer[total_bytes_written],std::min(static_cast<size_t>(MY_BUFF_SIZE),bufferSize - total_bytes_written));
        base_utility::CHECK(bytes_written,"Bytes written returned -1 in my_write()");

        total_bytes_written += bytes_written;
    }
}

/**
 * Recieves data from fd to buffer until bufferSize value is met
*/
size_t base_utility::my_read(const int &fd,uint8_t buffer[],const size_t &bufferSize){
    size_t total_bytes_read = 0;

    while(total_bytes_read != bufferSize){
        ssize_t bytes_read = ::read(fd, &buffer[total_bytes_read],std::min(static_cast<size_t>(MY_BUFF_SIZE),bufferSize - total_bytes_read));
        base_utility::CHECK(bytes_read,"Bytes read returned -1 in my_read()");

        if(bytes_read == 0){
            return total_bytes_read;
        }

        total_bytes_read += bytes_read;
    }

    return total_bytes_read;   
}


void base_utility::write_UInt(const int &fd,size_t &data){
    auto send_buf_size = static_cast<uint8_t*>(static_cast<void*>(&data));
    base_utility::my_write(fd,send_buf_size,sizeof(data));
}

void base_utility::write_String(const int &fd,const std::string &buf)
{
    // https://stackoverflow.com/questions/5585532/c-int-to-byte-array

    //First send the length of the string. We will not be including the null terminator in the length
    size_t str_size = buf.size();
    base_utility::write_UInt(fd,str_size);
    
    //Then send the string
    auto str_data = static_cast<uint8_t*>(static_cast<void*>(const_cast<char*>(buf.data())));
    base_utility::my_write(fd,str_data,str_size);
}

std::tuple<size_t,size_t> base_utility::read_UInt(const int &fd){
    std::array<uint8_t,sizeof(size_t)> buffer_for_length{};
    auto bytes_read = base_utility::my_read(fd,buffer_for_length.data(),sizeof(size_t));
    
    // https://stackoverflow.com/questions/52492229/c-byte-array-to-int
    size_t data;
    memcpy(&data,buffer_for_length.data(),sizeof(size_t));
    return std::make_tuple(bytes_read,data);
}

std::tuple<size_t,std::array<uint8_t,STR_MAX_SIZE>> base_utility::read_String(const int &fd,const size_t &length){
    
    // The length will not be including the null terminator
    std::array<uint8_t,STR_MAX_SIZE> buffer_for_string{};

    // string.size() -> Returns the number of characters in the string, not including any null-termination.
    auto bytes_read = base_utility::my_read(fd,buffer_for_string.data(),length);

    // Max string content length can be 255{0-254} and last{255} is for null terminator
    if(length >= STR_MAX_SIZE-1){
        // In this block length is either 255 or more than 255
        buffer_for_string.data()[STR_MAX_SIZE-1] = '\0';
    }else{
        // In this block length is less than 255. It might can be 254 also
        buffer_for_string.data()[length+1] = '\0';
    }

    return std::make_tuple(bytes_read,buffer_for_string);
}