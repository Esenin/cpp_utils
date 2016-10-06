cmake_minimum_required(VERSION 2.8)
project(test_task)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11")

file(GLOB PRJ_HDRS "${CMAKE_SOURCE_DIR}/src/*.h")
file(GLOB PRJ_SRC "${CMAKE_SOURCE_DIR}/src/*.cpp")
list(REMOVE_ITEM PRJ_SRC main.cpp)

file(GLOB TEST_HDRS "${CMAKE_SOURCE_DIR}/test/*.h")
file(GLOB TEST_SRC "${CMAKE_SOURCE_DIR}/test/*.cpp")

enable_testing()
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
find_package(Threads REQUIRED)


add_library(app_code ${PRJ_HDRS} ${PRJ_SRC})

add_executable(filter_n_sort "${CMAKE_SOURCE_DIR}/src/main.cpp")
target_link_libraries(filter_n_sort app_code)

add_executable(unit_tests ${TEST_HDRS} ${TEST_SRC})
target_link_libraries(unit_tests gtest gtest_main)
target_link_libraries(unit_tests ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(unit_tests app_code)

add_test(all_tests unit_tests)


