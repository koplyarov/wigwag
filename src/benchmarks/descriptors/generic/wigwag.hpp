#ifndef BENCHMARKS_DESCRIPTORS_GENERIC_WIGWAG_HPP
#define BENCHMARKS_DESCRIPTORS_GENERIC_WIGWAG_HPP


#include <wigwag/life_token.hpp>

#include <string>


namespace descriptors {
namespace generic {
namespace wigwag
{

	struct life_token
	{
		using Type = ::wigwag::life_token;
		static ::std::string GetName() { return "life_token"; }
	};


}}}

#endif
