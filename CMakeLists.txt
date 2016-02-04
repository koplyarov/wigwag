project(wigwag)
cmake_minimum_required(VERSION 2.6)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

include_directories(${CMAKE_SOURCE_DIR})

add_definitions(-std=c++11)
add_definitions(-Wall)
if (WIGWAG_VALGRIND)
	add_definitions(-DWIGWAG_USE_HELGRIND_ANNOTATIONS=1)
endif()

######################################################################

if (WIGWAG_COVERAGE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
endif()

find_package(Threads)

find_package(CxxTest)
if(CXXTEST_FOUND)
    include_directories(${CXXTEST_INCLUDE_DIR})
    enable_testing()
    CXXTEST_ADD_TEST(wigwag_test unittest_wigwag.cpp
                      ${CMAKE_CURRENT_SOURCE_DIR}/test/test.hpp)
    target_link_libraries(wigwag_test ${CMAKE_THREAD_LIBS_INIT}) # as needed
endif()
