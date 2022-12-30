cmake_minimum_required(VERSION 3.22.1 FATAL_ERROR)

project(file_server LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(my_base)
add_subdirectory(tcp_server)
add_subdirectory(tcp_client)
add_subdirectory(driver)

