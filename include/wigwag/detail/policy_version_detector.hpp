#ifndef WIGWAG_DETAIL_POLICY_VERSION_DETECTOR_HPP
#define WIGWAG_DETAIL_POLICY_VERSION_DETECTOR_HPP

// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <type_traits>


namespace wigwag {
namespace detail
{

#include <wigwag/detail/disable_warnings.hpp>

	template < typename... VersionChecks_ >
	struct policy_version_detector
	{
		using adapted_policy = void;
	};


	template < typename HeadVersionCheck_, typename... TailVersionChecks_ >
	struct policy_version_detector<HeadVersionCheck_, TailVersionChecks_...>
	{
		using adapted_policy = typename std::conditional<
				std::is_same<typename HeadVersionCheck_::adapted_policy, void>::value,
				typename policy_version_detector<TailVersionChecks_...>::adapted_policy,
				typename HeadVersionCheck_::adapted_policy
			>::type;
	};

#include <wigwag/detail/enable_warnings.hpp>

}}

#endif
