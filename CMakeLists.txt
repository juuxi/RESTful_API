cmake_minimum_required(VERSION 3.10)

project(RESTful_API)

set(SERVER_SOURCE_DIR server)
set(CLIENT_SOURCE_DIR client)

include_directories(server)
include_directories(client)
include_directories(common)
#include_directories(/usr/include/postgresql)

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -I/usr/include/postgresql")

add_executable(server ${SERVER_SOURCE_DIR}/server.cpp ${SERVER_SOURCE_DIR}/postgreconn.hpp)
add_executable(client ${CLIENT_SOURCE_DIR}/client.cpp)

find_package(PostgreSQL)
target_link_libraries(server PRIVATE PostgreSQL::PostgreSQL)
