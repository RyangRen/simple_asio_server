cmake_minimum_required(VERSION 3.0)

project(simple_server)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_executable(simple_server simple_server.cpp)

target_link_libraries(simple_server pthread)