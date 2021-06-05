cmake_minimum_required(VERSION 3.8)

include_directories(../)

add_executable(
        MTServer
        mtserver.cpp
)

# 链接阶段
target_link_libraries(MTServer FreeCPlus)
target_link_libraries(MTServer pthread)
