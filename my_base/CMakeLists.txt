add_library(mybase src/base.cpp)

target_include_directories(mybase
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/libs
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(mybase PRIVATE uuid)
# uuid will link as: $ gcc base.cpp -o base.o -luuid