cmake_minimum_required(VERSION 2.8)
project(threadsafe_hashmap)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

find_package(Threads REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

set(SOURCE_FILES main.cpp include/threadsafe_hashmap.h src/concurrent_linked_list.h tests/linked_list_test.h tests/concurrent_list_test.h tests/concurrent_map_test.h)
add_executable(threadsafe_hashmap ${SOURCE_FILES})
target_link_libraries(threadsafe_hashmap ${CMAKE_THREAD_LIBS_INIT})