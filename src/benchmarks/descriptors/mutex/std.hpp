#ifndef BENCHMARKS_DESCRIPTORS_THREADING_STD_HPP
#define BENCHMARKS_DESCRIPTORS_THREADING_STD_HPP


#include <condition_variable>
#include <mutex>
#include <string>


namespace descriptors {
namespace mutex {
namespace std
{

	struct Mutex
	{
		using MutexType = ::std::mutex;
		static ::std::string GetName() { return "std"; }
	};

	struct RecursiveMutex
	{
		using MutexType = ::std::recursive_mutex;
		static ::std::string GetName() { return "std_recursive"; }
	};

}}}

#endif
