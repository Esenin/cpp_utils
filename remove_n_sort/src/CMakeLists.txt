cmake_minimum_required(VERSION 2.8)
project(remove_n_sort)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11")

set(SOURCE_FILES main.cpp filter_base.h sorted_storage_base.h application.h simple_filter.cpp simple_filter.h simple_storage.cpp simple_storage.h)
add_executable(remove_n_sort ${SOURCE_FILES})