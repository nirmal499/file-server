cmake_minimum_required(VERSION 3.22.1 FATAL_ERROR)

project(file_server LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

# 0 :- Nothing
# 1 :- echo server single connection
# 2 :- file server single connection
# 3 :- multi threaded file echo server
# 4 :- echo server handled by select
# 5 :- echo server handled by poll
# 6 :- echo server handled by epoll

add_compile_definitions(SERVER_TYPE=3)

add_subdirectory(my_base)
add_subdirectory(my_pool)
add_subdirectory(tcp_server)
add_subdirectory(tcp_client)
add_subdirectory(driver)

