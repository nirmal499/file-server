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

    void myclient::start(const char *config_file)
    {

        auto [parsed_string, files_vec] = this->parse_for_both_uploading_and_downloading(config_file);

        base_utility::write_String(m_fd, parsed_string);

        // cout << "Parsed_String is : " << parsed_string << '\n';

        if (parsed_string[0] == '1')
        {
            /***client wants to upload files*/
            /***server will be downloading files*/

            // tuple<"old_file_name","new_file_name_from_server">
            std::vector<std::tuple<std::string, std::string>> new_file_name_details_vec;
            int unsuccessful_upload_count = 0;
            for (auto &file : files_vec)
            {
                /**Here we will be sending file*/

                size_t fileSize_written = base_utility::sendFile(m_fd, file);
                if (fileSize_written == 0)
                {
                    cout << "Some errored during sending/writing/uploading file " << file << " as the sent fileSize is 0\n";
                    unsuccessful_upload_count++;
                }
                else
                {
                    auto [bytes_read, new_file_name_from_server_arr_of_uint8] = base_utility::read_String(m_fd);
                    string new_file_name_from_server(reinterpret_cast<char *>(new_file_name_from_server_arr_of_uint8.data()));

                    new_file_name_details_vec.emplace_back(std::make_tuple(std::move(file), std::move(new_file_name_from_server)));
                }
            }

            cout << "\nRESULTS: \n1. NO.OF UPLOADED FILES : " << files_vec.size() - unsuccessful_upload_count;
            cout << "\n2. NO.OF UNSUCCESSFUL UPLOADS : " << unsuccessful_upload_count << "\n";

            cout << "\nNEW FILE NAME DETAILS\n";
            for (auto &file_name_tuple : new_file_name_details_vec)
            {
                cout << get<0>(file_name_tuple) << " ---> " << get<1>(file_name_tuple) << "\n";
            }
        }
        else if (parsed_string[0] == '2')
        {
            /**client wants to download files*/
            /**server will be uploading files*/
            int unsuccessful_download_count = 0;
            int error_occured_count = 0;
            size_t fileSize_read{};
            for (const auto &file_name : files_vec)
            {
                auto [bytes_read, response_uint] = base_utility::read_UInt(m_fd);
                cout << "\nRESULT FOR FILE: " << file_name << "\n";

                if (bytes_read == 0)
                {
                    cout << "Bytes_read is 0 from the server during reading the response_uint for this file\n";
                    error_occured_count++;
                }
                else
                {
                    if (response_uint == 100)
                    {
                        cout << "Error occurred at server.Please try again for this file..\n";
                        error_occured_count++;
                    }
                    else if (response_uint == 200)
                    {
                        cout << "This file does not exists in the server..\n";
                        error_occured_count++;
                    }
                    else if (response_uint == 300)
                    {
                        cout << "You are not allowed to download this file\n";
                        error_occured_count++;
                    }
                    else if (response_uint == 400)
                    {
                        cout << "Downloading the FILE........\n";
                        fileSize_read = base_utility::recvFile(m_fd, CLIENT_STORAGE_FOLDER + file_name);

                        if (fileSize_read == 0)
                        {
                            cout << "Some errored during receving/reading/downloading this file as the sent fileSize is 0\n";
                            unsuccessful_download_count++;
                        }else{
                            cout << "FILE is downloaded.\n";
                        }
                    }
                }
            }

            cout << "\nRESULTS: \n1. NO.OF DOWNLOADED FILES : " << files_vec.size() - error_occured_count - unsuccessful_download_count;
            cout << "\n2. NO.OF UNSUCCESSFUL DOWNLOADS : " << error_occured_count + unsuccessful_download_count << "\n";
        }
        else
        {
            cout << "Undefined operation....\n";
        }
    }

    tuple<string, vector<string>> myclient::parse_for_both_uploading_and_downloading(const string &config_file)
    {

        int operation;
        vector<string> files_vec;
        string username, permission_users, file, templine, final_string;
        size_t first_comma_pos{};

        ifstream infile(config_file);
        if (infile.is_open())
        {
            // here ':' is the delimitor
            getline(infile, templine, ':');
            infile >> operation;
            infile.get(); /* Discards '\n' */

            getline(infile, templine, ':');
            infile >> username;
            (void)infile.get(); /* Discards '\n' */

            final_string = to_string(operation) + '@' + username + '$';

            if (operation == 1)
            {
                /***client wants to upload files***/
                while (getline(infile, templine))
                {
                    // this while loop will loop until EOF is encountered

                    if (templine.empty())
                    {
                        // Incase if we have a empty line
                        break;
                    }

                    first_comma_pos = templine.find_first_of(',');
                    // first_comma_pos is not included in permission_users string
                    permission_users = templine.substr(0, first_comma_pos);

                    file = templine.substr(first_comma_pos + 1);

                    // We need the absolute path of the file for uploading to the server
                    files_vec.push_back(file);

                    file = file.substr(file.find_last_of('/') + 1);

                    final_string += permission_users + '#' + file + '&';
                }
            }
            else if (operation == 2)
            {
                /***client wants to download files*****/
                while (getline(infile, templine))
                {

                    if (templine.empty())
                    {
                        break;
                    }

                    final_string += templine + '&';
                    files_vec.emplace_back(move(templine));
                }
            }
            else
            {
                /***Invalid operation*****/
                throw runtime_error("Undefined Operation stated by the passed config_file..\n");
            }
        }
        else
        {
            throw runtime_error("Error in opening config file..\n");
        }

        return make_tuple(move(final_string), move(files_vec));
    }

    void myclient::echo()
    {
        while (true)
        {
            string sendbuf{};

            // cin >> sendbuf;
            getline(cin, sendbuf);

            base_utility::write_String(m_fd, sendbuf);
            auto [bytes_read, my_str] = base_utility::read_String(m_fd);

            if (bytes_read == 0)
            {
                break;
            }
            cout << my_str.data() << "\n";
        }
    }

}