#ifndef BENCHMARKS_DESCRIPTORS_FUNCTIONS_BOOST_HPP
#define BENCHMARKS_DESCRIPTORS_FUNCTIONS_BOOST_HPP


#include <boost/function.hpp>

#include <string>


namespace descriptors {
namespace function {
namespace boost
{

	struct Regular
	{
		using FunctionType = ::boost::function<void()>;

		static std::string GetName() { return "boost"; }
	};

}}}

#endif
