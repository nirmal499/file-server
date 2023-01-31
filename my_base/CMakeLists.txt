add_library(mybase src/base.cpp)

target_include_directories(mybase
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/libs
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}/include")

if(DEFINED DDBFILE AND DEFINED SERVERPATH AND DEFINED CLIENTPATH)
    target_compile_definitions(mybase
        PRIVATE CONFIDENTIAL_FILE=${DDBFILE}
        PRIVATE SERVER_STORAGE_FOLDER=${SERVERPATH}
        PRIVATE CLIENT_STORAGE_FOLDER=${CLIENTPATH}
    )
endif()
unset(DDBFILE CACHE)
unset(SERVERPATH CACHE)
unset(CLIENTPATH CACHE)

target_link_libraries(mybase PRIVATE uuid)
# uuid will link as: $ gcc base.cpp -o base.o -luuid