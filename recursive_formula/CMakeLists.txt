cmake_minimum_required(VERSION 2.8)
project(RecursiveFunction)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE  "${CMAKE_CXX_FLAGS_RELEASE} -O3")

set(SOURCE_FILES src/main.cpp src/solvers.h src/helpers.h)

add_executable(RecursiveFunction ${SOURCE_FILES})