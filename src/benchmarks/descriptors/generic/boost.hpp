#ifndef BENCHMARKS_DESCRIPTORS_GENERIC_BOOST_HPP
#define BENCHMARKS_DESCRIPTORS_GENERIC_BOOST_HPP


#include <boost/thread/condition_variable.hpp>

#include <string>


namespace descriptors {
namespace generic {
namespace boost
{

	struct condition_variable
	{
		using Type = ::boost::condition_variable;
		static std::string GetName() { return "boost_condition_variable"; }
	};


}}}

#endif
