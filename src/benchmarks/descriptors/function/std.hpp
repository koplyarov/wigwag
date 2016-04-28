#ifndef BENCHMARKS_DESCRIPTORS_FUNCTIONS_STD_HPP
#define BENCHMARKS_DESCRIPTORS_FUNCTIONS_STD_HPP


#include <functional>
#include <string>


namespace descriptors {
namespace function {
namespace std
{

	struct Regular
	{
		using FunctionType = ::std::function<void()>;

		static ::std::string GetName() { return "std"; }
	};

}}}

#endif
