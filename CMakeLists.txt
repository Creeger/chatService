cmake_minimum_required(VERSION 3.16)

project(chatServ LANGUAGES C)

set(CMAKE_C_STANDARD 99)

# Main C executable
add_executable(chatServ src/server.c)

add_executable(chatClient src/client.c src/getIpAddr.c)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

