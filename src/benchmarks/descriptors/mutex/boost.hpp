#ifndef BENCHMARKS_DESCRIPTORS_THREADING_BOOST_HPP
#define BENCHMARKS_DESCRIPTORS_THREADING_BOOST_HPP


#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>

#include <string>


namespace descriptors {
namespace mutex {
namespace boost
{

	struct mutex
	{
		using MutexType = ::boost::mutex;
		static std::string GetName() { return "boost"; }
	};

	struct recursive_mutex
	{
		using MutexType = ::boost::recursive_mutex;
		static std::string GetName() { return "boost_recursive"; }
	};


}}}

#endif
