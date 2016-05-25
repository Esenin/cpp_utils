cmake_minimum_required(VERSION 2.8)
project(threadsafe_hashmap)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

set(SOURCE_FILES main.cpp include/threadsafe_hashmap.h src/bucket.h src/linked_list.h tests/linked_list_test.h)
add_executable(threadsafe_hashmap ${SOURCE_FILES})