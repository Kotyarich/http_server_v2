cmake_minimum_required(VERSION 3.10)
project(server)

set(CMAKE_CXX_STANDARD 17)

add_executable(server main.cpp Server.cpp Server.h Worker.cpp Worker.h HttpParser.cpp HttpParser.h HttpUtils.cpp HttpUtils.h Exception.cpp Exception.h)
target_link_libraries(${PROJECT_NAME} stdc++fs)