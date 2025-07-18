cmake_minimum_required(VERSION 3.10)

project(RESTful_API)

set(SERVER_SOURCE_DIR server)
set(CLIENT_SOURCE_DIR client)
set(COMMON_SOURCE_DIR common)

include_directories(server)
include_directories(client)
include_directories(common)

add_executable(server ${SERVER_SOURCE_DIR}/server.cpp ${COMMON_SOURCE_DIR}/json.hpp)

add_executable(client ${CLIENT_SOURCE_DIR}/client.cpp ${COMMON_SOURCE_DIR}/json.hpp)


add_library(db ${SERVER_SOURCE_DIR}/db.hpp ${SERVER_SOURCE_DIR}/db.cpp 
${SERVER_SOURCE_DIR}/postgreconn.hpp ${SERVER_SOURCE_DIR}/postgreconn.cpp ${COMMON_SOURCE_DIR}/json.hpp)
target_link_libraries(server PRIVATE db)
#target_link_directories(server PRIVATE ${SERVER_SOURCE_DIR}/dp.hpp)

find_package(PostgreSQL)
target_link_libraries(db PRIVATE PostgreSQL::PostgreSQL)
target_link_libraries(server PRIVATE PostgreSQL::PostgreSQL)
