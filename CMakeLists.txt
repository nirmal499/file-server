cmake_minimum_required(VERSION 3.22.1 FATAL_ERROR)

project(file_server LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

# SERVER_TYPE-> 0 :- Nothing [DEFAULT]
# SERVER_TYPE-> 1 :- echo server single connection
# SERVER_TYPE-> 2 :- file server single connection
# SERVER_TYPE-> 3 :- multi threaded file echo server
# SERVER_TYPE-> 4 :- echo server handled by select
# SERVER_TYPE-> 5 :- echo server handled by poll
# SERVER_TYPE-> 6 :- echo server handled by epoll

if(DEFINED SERVERT)
    add_compile_definitions(SERVER_TYPE=${SERVERT})
    # message(STATUS "SERVERT=${SERVERT}")
endif()
unset(SERVERT CACHE)

# message(STATUS ${SERVERT})

add_subdirectory(my_base)
add_subdirectory(my_pool)
add_subdirectory(tcp_server)
add_subdirectory(tcp_client)
add_subdirectory(driver)

