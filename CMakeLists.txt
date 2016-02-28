project(wigwag)
cmake_minimum_required(VERSION 2.6)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

if (WIGWAG_VALGRIND)
	message(STATUS "Using valgrind annotations")
	add_definitions(-DWIGWAG_USE_HELGRIND_ANNOTATIONS=1)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -include wigwag/detail/stdcpp_annotations.hpp")
endif()

if (MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
else()
	add_definitions(-Wall)
	add_definitions(-std=c++11)
endif()

if (WIGWAG_COVERAGE)
	message(STATUS "Using gcov for API_V1_test coverage analysis")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -include wigwag/detail/coverage.hpp")
endif()

find_package(Threads)

######################################################################

if (MSVC)
	set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/test/no_exceptions_compilation_test.cpp PROPERTIES COMPILE_FLAGS
		"/W4 /WX /D_HAS_EXCEPTIONS=0")
	get_source_file_property(TMP_COMPILE_FLAGS ${CMAKE_CURRENT_SOURCE_DIR}/src/test/no_exceptions_compilation_test.cpp COMPILE_FLAGS)
	string(REPLACE "/EHsc" "/EHs-c-" TMP_COMPILE_FLAGS ${TMP_COMPILE_FLAGS})
	set_source_files_properties(myprogram.cpp COMPILE_FLAGS ${TMP_COMPILE_FLAGS})

	set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/test/pedantic_compilation_test.cpp PROPERTIES COMPILE_FLAGS
		"/W4 /WX")
else()
	set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/test/no_exceptions_compilation_test.cpp PROPERTIES COMPILE_FLAGS
			"-Werror=all -Werror=extra -Werror=effc++ -Werror=pedantic -pedantic -pedantic-errors -fno-exceptions -fno-rtti -fcheck-new")
	set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/test/pedantic_compilation_test.cpp PROPERTIES COMPILE_FLAGS
			"-Werror=all -Werror=extra -Werror=effc++ -Werror=pedantic -pedantic -pedantic-errors")
endif()

add_library(wigwag_pedantic_compilation_test
	${CMAKE_CURRENT_SOURCE_DIR}/src/test/no_exceptions_compilation_test.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/test/pedantic_compilation_test.cpp)
target_link_libraries(wigwag_pedantic_compilation_test)

######################################################################

add_library(example_common ${CMAKE_CURRENT_SOURCE_DIR}/examples/common/Logger.cpp)
target_link_libraries(example_common ${CMAKE_THREAD_LIBS_INIT})

add_executable(example_observable_dictionary ${CMAKE_CURRENT_SOURCE_DIR}/examples/observable_dictionary/ObservableDictionaryExample.cpp)
target_link_libraries(example_observable_dictionary example_common)

######################################################################

find_package(CxxTest)
if(CXXTEST_FOUND)
	message(STATUS "Found cxxtest, enabling wigwag_test")
	include_directories(${CXXTEST_INCLUDE_DIR})
	enable_testing()

	CXXTEST_ADD_TEST(wigwag_test unittest_wigwag.cpp
					  ${CMAKE_CURRENT_SOURCE_DIR}/src/test/api_v1_test.hpp)

	if (MSVC)
		set_source_files_properties(unittest_wigwag.cpp PROPERTIES COMPILE_FLAGS "/wd4800")
	endif()

	target_link_libraries(wigwag_test ${CMAKE_THREAD_LIBS_INIT})
endif()

######################################################################

find_package(Boost COMPONENTS
	program_options
	system
	exception
	thread
	date_time
)

if (Boost_FOUND)
	message(STATUS "Found boost, enabling wigwag_benchmarks")
	include_directories(${Boost_INCLUDE_DIRS})

	add_executable(wigwag_benchmarks ${CMAKE_CURRENT_SOURCE_DIR}/src/benchmarks/benchmarks.cpp)

	target_link_libraries(wigwag_benchmarks
		${Boost_LIBRARIES}
		${CMAKE_THREAD_LIBS_INIT}
	)
endif()
