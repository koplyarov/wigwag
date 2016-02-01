project(wigwag)
cmake_minimum_required(VERSION 2.6)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

include_directories(${CMAKE_SOURCE_DIR})

add_definitions(-std=c++11)
add_definitions(-Wall)

######################################################################

set(WIGWAG_TEST_SRCS test/test.cpp)

find_package(Threads)

add_executable(wigwag_test ${WIGWAG_TEST_SRCS})
target_link_libraries(wigwag_test ${CMAKE_THREAD_LIBS_INIT})
