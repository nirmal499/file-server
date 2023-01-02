#include "../include/my_base/base.hpp"
#include <algorithm>
#include <fstream>

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

std::tuple<size_t,std::array<uint8_t,STR_MAX_SIZE>> base_utility::read_String(const int &fd){
    
    // The length will not be including the null terminator
    std::array<uint8_t,STR_MAX_SIZE> buffer_for_string{};

    auto [bytes_read1,length_of_string_as_data] = base_utility::read_UInt(fd);
    if(bytes_read1 == 0){
        // here buffer_for_string is empty
        return std::make_tuple(bytes_read1,buffer_for_string);
    }

    // string.size() -> Returns the number of characters in the string, not including any null-termination.
    auto bytes_read2 = base_utility::my_read(fd,buffer_for_string.data(),length_of_string_as_data);
    if(bytes_read2 == 0){
        // here buffer_for_string is empty
        return std::make_tuple(bytes_read2,buffer_for_string);
    }

    // Max string content length can be 255{0-254} and last{255} is for null terminator
    if(length_of_string_as_data >= STR_MAX_SIZE-1){
        // In this block length is either 255 or more than 255
        buffer_for_string.data()[STR_MAX_SIZE-1] = '\0';
    }else{
        // In this block length is less than 255. It might can be 254 also
        buffer_for_string.data()[length_of_string_as_data+1] = '\0';
    }
    /**
     * bytes_read1, bytes_read2 and length_of_string_as_data all will be same
    */
    return std::make_tuple(bytes_read2,buffer_for_string);
}

size_t base_utility::sendFile(const int &fd, const string &fileName){
    
    std::array<uint8_t,MY_BUFF_SIZE> buffer;

    ifstream uploadfile(fileName,ios_base::binary);

    if(!uploadfile.is_open()){
        return 0;
    }

    uploadfile.seekg(0,ios_base::end);
    size_t fileSize = uploadfile.tellg();
    uploadfile.seekg(0,ios_base::beg);

    base_utility::write_UInt(fd,fileSize);

    size_t total_bytes_to_be_send = fileSize;

    while(total_bytes_to_be_send != 0){
        size_t to_read_bytes = min(total_bytes_to_be_send,static_cast<size_t>(MY_BUFF_SIZE));

        if(!uploadfile.read(reinterpret_cast<char *>(buffer.data()),to_read_bytes)){
            return 0;
        }

        base_utility::my_write(fd,buffer.data(),to_read_bytes);

        total_bytes_to_be_send -= to_read_bytes;
    }

    uploadfile.close();  

    return fileSize;
}

size_t base_utility::recvFile(const int &fd, const string &absolute_fileName){
    std::array<uint8_t,MY_BUFF_SIZE> buffer;

    ofstream outfile(absolute_fileName, ios_base::binary);

    if(!outfile.is_open()){
        return 0;
    }

    auto [bytes_read,fileSize] = base_utility::read_UInt(fd);
    if(bytes_read == 0){
        return 0;
    }

    size_t total_bytes_to_be_read = fileSize;

    while(total_bytes_to_be_read != 0){
        size_t to_write_bytes = base_utility::my_read(fd,buffer.data(),min(static_cast<size_t>(MY_BUFF_SIZE), total_bytes_to_be_read));

        if(to_write_bytes == 0 || !outfile.write(reinterpret_cast<char *>(buffer.data()),to_write_bytes)){
            return 0;
        }

        total_bytes_to_be_read -= to_write_bytes;
    }

    outfile.close();  

    return fileSize;
}
