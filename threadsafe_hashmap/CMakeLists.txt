cmake_minimum_required(VERSION 2.8)
project(threadsafe_hashmap)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -pg")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -DNDEBUG -fsanitize=thread -fPIE -pie -g -std=c++1y") # for clang sanitizer


set(SOURCE_FILES main.cpp include/threadsafe_hashmap.h src/bucket.h tests/bucket_test.h tests/concurrent_bucket_test.h tests/hashmap_test.h src/helpers.h)

find_package(Threads REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
add_executable(threadsafe_hashmap ${SOURCE_FILES})
target_link_libraries(threadsafe_hashmap ${CMAKE_THREAD_LIBS_INIT})