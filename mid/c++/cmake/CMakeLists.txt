cmake_minimum_required(VERSION 3.0)
set (CMAKE_CXX_STANDARD 20)

project(ant)
add_library(ant SHARED ../map.cpp ../ant.cpp ../job.cpp ../Worker.cpp ../driver/interfaces.cpp)

target_link_libraries(ant PUBLIC -latomic)
