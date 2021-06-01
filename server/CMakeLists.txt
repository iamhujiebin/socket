cmake_minimum_required(VERSION 3.8)

include_directories(../)

add_executable(
        Server
        server.cpp
)

target_link_libraries(Server FreeCPlus)