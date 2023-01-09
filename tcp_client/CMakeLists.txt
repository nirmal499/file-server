add_library(tcpclient src/tcp_client.cpp)

target_link_libraries(tcpclient PRIVATE mybase)

target_include_directories(tcpclient
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        mybase
)
