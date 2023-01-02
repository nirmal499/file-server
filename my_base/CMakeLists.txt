add_library(my_base src/base.cpp)

target_include_directories(my_base
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/libs
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(my_base PRIVATE uuid)
# uuid will link as: $ gcc base.cpp -o base.o -luuid