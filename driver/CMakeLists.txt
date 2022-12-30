add_executable(client client.cpp)
add_executable(server server.cpp)

target_include_directories(server
    PUBLIC tcpserver
)

target_link_libraries(server
    PUBLIC tcpserver
)

target_include_directories(client
    PUBLIC tcpclient
)

target_link_libraries(client
    PUBLIC tcpclient
)