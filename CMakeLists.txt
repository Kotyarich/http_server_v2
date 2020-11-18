cmake_minimum_required(VERSION 3.10)
project(server)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -lpqxx -lpq")
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(PQXX /usr/local/include/pqxx)

find_library(PQXX_LIB pqxx)
find_library(PQ_LIB pq)

add_executable(server main.cpp Server.cpp Server.h Worker.cpp Worker.h HttpRequest.cpp HttpRequest.h NetUtils.cpp NetUtils.h Exception.cpp Exception.h entities/User.h db/PostgresExtensionRepository.cpp db/PostgresExtensionRepository.h entities/Extension.h db/ExtensionRepository.h)
add_executable(client client/client.cpp)
target_link_libraries(${PROJECT_NAME} stdc++fs)
TARGET_LINK_LIBRARIES(${PROJECT_NAME}  ${PQXX_LIB} ${PQ_LIB})