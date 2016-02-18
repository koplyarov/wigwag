project(wigwag)
cmake_minimum_required(VERSION 2.6)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/src)

add_definitions(-std=c++11)
add_definitions(-Wall)
if (WIGWAG_VALGRIND)
	message(STATUS "Using valgrind annotations")
	add_definitions(-DWIGWAG_USE_HELGRIND_ANNOTATIONS=1)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -include wigwag/detail/stdcpp_annotations.hpp")
endif()

if (WIGWAG_COVERAGE)
	message(STATUS "Using gcov for test coverage analysis")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
endif()

find_package(Threads)

######################################################################

find_package(CxxTest)
if(CXXTEST_FOUND)
	include_directories(${CXXTEST_INCLUDE_DIR})
	enable_testing()
	CXXTEST_ADD_TEST(wigwag_test unittest_wigwag.cpp
					  ${CMAKE_CURRENT_SOURCE_DIR}/src/test/test.hpp)
	target_link_libraries(wigwag_test ${CMAKE_THREAD_LIBS_INIT})
endif()

######################################################################

find_package(Boost COMPONENTS
	program_options
	system
	exception
)

if (Boost_FOUND)
	include_directories(${Boost_INCLUDE_DIRS})

	add_executable(wigwag_benchmarks ${CMAKE_CURRENT_SOURCE_DIR}/src/benchmarks/benchmarks.cpp)

	target_link_libraries(wigwag_benchmarks
		${Boost_PROGRAM_OPTIONS_LIBRARY}
		${Boost_SYSTEM_LIBRARY}
		${Boost_EXCEPTION_LIBRARY}
		${CMAKE_THREAD_LIBS_INIT}
	)
endif()
