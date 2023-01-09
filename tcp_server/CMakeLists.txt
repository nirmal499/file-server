add_library(tcpserver src/tcp_server.cpp)

target_link_libraries(tcpserver 
    PRIVATE mybase
    PRIVATE threadpool
)

target_include_directories(tcpserver
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        mybase
    PRIVATE
        threadpool
)
