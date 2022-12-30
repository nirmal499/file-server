add_library(my_base src/base.cpp)

target_include_directories(my_base
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)