#ifndef WIGWAG_POLICIES_REF_COUNTER_POLICY_CONCEPT_HPP
#define WIGWAG_POLICIES_REF_COUNTER_POLICY_CONCEPT_HPP

// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


namespace wigwag {
namespace detail {
namespace ref_counter
{

#include <wigwag/detail/disable_warnings.hpp>

	WIGWAG_DECLARE_TYPE_EXPRESSION_CHECK(has_ctor, T_(0));
	WIGWAG_DECLARE_TYPE_EXPRESSION_CHECK(has_add_ref, std::declval<T_>().add_ref() == 0);
	WIGWAG_DECLARE_TYPE_EXPRESSION_CHECK(has_release, std::declval<T_>().release() == 0);

	template < typename T_ >
	struct check_policy_v1_0
	{
		using adapted_policy = typename std::conditional<has_ctor<T_>::value && has_add_ref<T_>::value && has_release<T_>::value, T_, void>::type;
	};


	template < typename T_ >
	struct policy_concept
	{
		using adapted_policy = typename wigwag::detail::policy_version_detector<check_policy_v1_0<T_>>::adapted_policy;
	};

#include <wigwag/detail/enable_warnings.hpp>

}}}

#endif
