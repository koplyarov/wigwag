#ifndef BENCHMARKS_DESCRIPTORS_GENERIC_STD_HPP
#define BENCHMARKS_DESCRIPTORS_GENERIC_STD_HPP


#include <condition_variable>
#include <string>


namespace descriptors {
namespace generic {
namespace std
{

	struct ConditionVariable
	{
		using Type = ::std::condition_variable;
		static ::std::string GetName() { return "std_condition_variable"; }
	};


}}}

#endif
