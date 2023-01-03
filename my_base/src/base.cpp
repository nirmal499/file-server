#include <my_base/base.hpp>
#include <algorithm>
#include <fstream>

/**
 *  Writes data to fd from buffer until bufferSize value is met
*/
void base_utility::my_write(const int &fd,const uint8_t buffer[],const size_t &bufferSize){
    size_t total_bytes_written = 0;
    ssize_t bytes_written{};

    while (total_bytes_written != bufferSize)
    {
        bytes_written = ::write(fd, &buffer[total_bytes_written],std::min(static_cast<size_t>(MY_BUFF_SIZE),bufferSize - total_bytes_written));
        base_utility::CHECK(bytes_written,"Bytes written returned -1 in my_write()");

        total_bytes_written += bytes_written;
    }
}

/**
 * Recieves data from fd to buffer until bufferSize value is met
*/
size_t base_utility::my_read(const int &fd,uint8_t buffer[],const size_t &bufferSize){
    size_t total_bytes_read = 0;
    ssize_t bytes_read{};

    while(total_bytes_read != bufferSize){
        bytes_read = ::read(fd, &buffer[total_bytes_read],std::min(static_cast<size_t>(MY_BUFF_SIZE),bufferSize - total_bytes_read));
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
    size_t data{};
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


size_t base_utility::sendFile(const int &fd, const string &absolute_path_fileName){
    
    std::array<uint8_t,MY_BUFF_SIZE> buffer;

    ifstream uploadfile(absolute_path_fileName,ios_base::binary);

    if(!uploadfile.is_open()){
        return 0;
    }

    uploadfile.seekg(0,ios_base::end);
    size_t fileSize = uploadfile.tellg();
    uploadfile.seekg(0,ios_base::beg);

    base_utility::write_UInt(fd,fileSize);

    size_t total_bytes_to_be_send = fileSize;
    size_t to_read_bytes{};

    while(total_bytes_to_be_send != 0){
        to_read_bytes = min(total_bytes_to_be_send,static_cast<size_t>(MY_BUFF_SIZE));

        if(!uploadfile.read(reinterpret_cast<char *>(buffer.data()),to_read_bytes)){
            return 0;
        }

        base_utility::my_write(fd,buffer.data(),to_read_bytes);

        total_bytes_to_be_send -= to_read_bytes;
    }

    uploadfile.close();  

    return fileSize;
}

size_t base_utility::recvFile(const int &fd, const string &absolute_path_fileName){
    std::array<uint8_t,MY_BUFF_SIZE> buffer;

    ofstream outfile(absolute_path_fileName, ios_base::binary);

    if(!outfile.is_open()){
        return 0;
    }

    auto [bytes_read,fileSize] = base_utility::read_UInt(fd);
    if(bytes_read == 0){
        return 0;
    }

    size_t total_bytes_to_be_read = fileSize;
    size_t to_write_bytes{};

    while(total_bytes_to_be_read != 0){
        to_write_bytes = base_utility::my_read(fd,buffer.data(),min(static_cast<size_t>(MY_BUFF_SIZE), total_bytes_to_be_read));

        if(to_write_bytes == 0 || !outfile.write(reinterpret_cast<char *>(buffer.data()),to_write_bytes)){
            return 0;
        }

        total_bytes_to_be_read -= to_write_bytes;
    }

    outfile.close();  

    return fileSize;
}

std::string base_utility::generate_uuid(){
    // https://stackoverflow.com/questions/51053568/generating-a-random-uuid-in-c
    uuid_t bin_uuid;
    /*
     * Generate a UUID. We're not done yet, though,
     * for the UUID generated is in binary format 
     * (hence the variable name). We must 'unparse' 
     * binuuid to get a usable 36-character string.
     */

    // Under the hood the uuid_t is char[]
    uuid_generate_random(bin_uuid);

    /*
     * uuid_unparse() doesn't allocate memory for itself, so do that with
     * malloc(). 37 is the length of a UUID (36 characters), plus '\0'.
     */
    std::unique_ptr<std::array<char,37>> str_uuid = std::make_unique<std::array<char,37>>();
    if(str_uuid == nullptr){
        throw std::runtime_error("Error in allocating in heap make_unique..\n");
    }

    /* Produces a UUID string at uuid consisting of lower-case letters. */
    // Since str_uuid.get() gives a raw pointer so, thats why we need to use '->' instead of '.'
    uuid_unparse_lower(bin_uuid, str_uuid.get()->data());

    string my_str_uuid(str_uuid.get()->data());
    return my_str_uuid;
}
/**
 * It returns -1 if the internal file was not able to be opened
 * It returns 0 means the file_name passed does not exists in the DB
 * It returns 1 means the user_name passed is not permitted to access the file_name
 * It returns 2 means the user_name passed is permitted to access the file_name
*/
int base_utility::is_permitted(const string &file_name,const string &user_name){
    ifstream infile(CONFIDENTIAL_FILE,ios_base::in);

    if(!infile.is_open()){
        return -1;
    }

    string temp_string;
    // 0 means files does not exists
    // 1 means it is not permitted
    // 2 means it is permitted
    int flag_for_permitted = 1;
    size_t pos_of_separator{};
    string key_file_name{};
    string value_permission_str{};
    vector<string> permitted_users_vec{};

    while(getline(infile,temp_string)){
        if(temp_string.empty()){
            break;
        }

        pos_of_separator = temp_string.find_first_of("<====>");
        // pos_of_separator is not included in key_name string
        key_file_name = temp_string.substr(0,pos_of_separator);

        value_permission_str = temp_string.substr(pos_of_separator+1);

        if(key_file_name == file_name){
            permitted_users_vec = base_utility::split(value_permission_str,';');
            if(find(permitted_users_vec.begin(),permitted_users_vec.end(),user_name) != permitted_users_vec.end()){
                flag_for_permitted = 2;
                break;
            }
        }
        flag_for_permitted = 0;

    }

    return flag_for_permitted;

}

void base_utility::write_to_db(const std::string &key,const std::string &value){
    ofstream outfile(CONFIDENTIAL_FILE,ios_base::app);

    if(!outfile.is_open()){
        throw runtime_error("Error opening the confidential file..\n");
    }

    string line_to_append = key + "<====>" + value + '\n';
    outfile.write(line_to_append.data(),line_to_append.size());
}


/***
 * Returns a vector containing string after splitting the passed string 'str' as per the passed char delimiter 'delim'
*/
std::vector<std::string> base_utility::split(const string &str, char delim)
{
    vector<string> result{};
    stringstream ss(str);
    string item;

    while (getline(ss, item, delim))
    {
        result.push_back(item);
    }

    return result;
}

