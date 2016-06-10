cmake_minimum_required(VERSION 2.8)
project(ExternalSort)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

set(SOURCE_FILES src/main.cpp src/helpers/environment.cpp src/helpers/environment.h src/bounded_sorter.cpp src/bounded_sorter.h src/helpers/shared_file.h src/dynamic_chunk.cpp src/dynamic_chunk.h src/helpers/uniq_line.h src/helpers/shared_file.cpp src/helpers/FileStorage.cpp src/helpers/FileStorage.h)

find_package(Threads REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")


add_executable(external_sort ${SOURCE_FILES})
target_link_libraries(external_sort ${CMAKE_THREAD_LIBS_INIT})