add_library(tcpclient src/tcp_client.cpp)

target_link_libraries(tcpclient PRIVATE my_base)

target_include_directories(tcpclient
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        my_base
)
