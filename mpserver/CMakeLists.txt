cmake_minimum_required(VERSION 3.8)

include_directories(../)

add_executable(
        MPServer
        mpserver.cpp
)

# 链接阶段
target_link_libraries(MPServer FreeCPlus)