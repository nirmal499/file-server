add_library(threadpool src/pool.cpp)

target_include_directories(
    threadpool PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
)