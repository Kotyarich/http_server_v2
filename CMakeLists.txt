cmake_minimum_required(VERSION 3.10)
project(server)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(server main.cpp Server.cpp Server.h Worker.cpp Worker.h HttpParser.cpp HttpParser.h NetUtils.cpp NetUtils.h Exception.cpp Exception.h)
target_link_libraries(${PROJECT_NAME} stdc++fs)