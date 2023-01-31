add_library(tcpserver src/tcp_server.cpp)

target_include_directories(tcpserver
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        mybase
    PRIVATE
        threadpool
)

if(DEFINED NTHREADS)
    target_compile_definitions(tcpserver PRIVATE NO_OF_THREADS=${NTHREADS})
endif()
unset(NTHREADS CACHE)

target_link_libraries(tcpserver 
    PRIVATE mybase
    PRIVATE threadpool
)

# get_target_property(COMPILE_DEFINITIONS tcpserver COMPILE_DEFINITIONS)
# message(STATUS "COMPILE_DEFINITIONS TCP_SERVER ${COMPILE_DEFINITIONS}")
