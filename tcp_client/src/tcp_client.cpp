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

    void myclient::start(const char *config_file){

        auto [parsed_string,files_vec] = this->parse(config_file);

        base_utility::write_String(m_fd,parsed_string);

        cout << "str is : " << parsed_string << '\n';

        int unsuccessful_upload_count = 0;
        for(auto &file: files_vec){
            /**Here we will be sending file*/

            size_t fileSize_written = base_utility::sendFile(m_fd,file);
            if(fileSize_written == 0){
                cout << "Some errored during sending/writing/uploading file " << file << " as the sent fileSize is 0\n";
                unsuccessful_upload_count++;
            }
        }

        cout << "\nRESULTS: \n1. NO.OF UPLOADED FILES : " << files_vec.size() - unsuccessful_upload_count;
        cout << "\n2. NO.OF UNSUCCESSFUL UPLOADS : " << unsuccessful_upload_count << "\n";

    }

    tuple<string, vector<string>> myclient::parse(const string &config_file){

        int operation;
        vector<string> files_vec;
        string username, permission_users, file, templine, final_string;

        ifstream infile(config_file);
        if(infile.is_open()){
            // here ':' is the delimitor
            getline(infile,templine, ':');
            infile >> operation;
            infile.get(); /* Discards '\n' */

            getline(infile, templine, ':');
            infile >> username;
            (void)infile.get(); /* Discards '\n' */

            final_string = to_string(operation) + '@' + username + '$';
            
            while(getline(infile,templine)){
                // this while loop will loop until EOF is encountered

                if(templine.empty()){
                    // Incase if we have a empty line
                    break;
                }

                size_t first_comma_pos = templine.find_first_of(',');
                permission_users = templine.substr(0, first_comma_pos);

                file = templine.substr(first_comma_pos + 1);
                
                files_vec.push_back(file);
                
                file = file.substr(file.find_last_of('/') + 1);
                
                final_string += permission_users + '#' + file + '&';
            }

        }else{
            throw runtime_error("Error in opening config file");
        }

        return make_tuple(move(final_string),move(files_vec));
    }


    void myclient::echo(){
        while(true){
            string sendbuf{};

            // cin >> sendbuf;
            getline(cin,sendbuf);

            base_utility::write_String(m_fd,sendbuf);
            auto [bytes_read,my_str] = base_utility::read_String(m_fd);
            
            if(bytes_read == 0){
                break;
            }
            cout << my_str.data() << "\n";
        }
    }

}