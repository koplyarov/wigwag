#ifndef WIGWAG_POLICIES_THREADING_POLICY_CONCEPT_HPP
#define WIGWAG_POLICIES_THREADING_POLICY_CONCEPT_HPP

// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <wigwag/detail/policy_version_detector.hpp>
#include <wigwag/detail/type_expression_check.hpp>


namespace wigwag {
namespace threading
{

#include <wigwag/detail/disable_warnings.hpp>

	namespace detail
	{
		WIGWAG_DECLARE_TYPE_EXPRESSION_CHECK(has_lock_primitive, std::declval<typename T_::lock_primitive>());
		WIGWAG_DECLARE_TYPE_EXPRESSION_CHECK(has_get_primitive, std::declval<T_>().get_primitive());
		WIGWAG_DECLARE_TYPE_EXPRESSION_CHECK(has_lock_recursive, std::declval<T_>().lock_recursive());
		WIGWAG_DECLARE_TYPE_EXPRESSION_CHECK(has_unlock_recursive, std::declval<T_>().unlock_recursive());
		WIGWAG_DECLARE_TYPE_EXPRESSION_CHECK(has_lock_nonrecursive, std::declval<T_>().lock_nonrecursive());
		WIGWAG_DECLARE_TYPE_EXPRESSION_CHECK(has_unlock_nonrecursive, std::declval<T_>().unlock_nonrecursive());

		template < typename T_, bool HasLockPrimitive_ = has_lock_primitive<T_>::value >
		struct check_lock_primitive_v1_0
		{
			using adapted_policy = void;
		};

		template < typename T_ >
		struct check_lock_primitive_v1_0<T_, true>
		{
			using lock_primitive = typename T_::lock_primitive;
			static const bool matches =
				has_get_primitive<lock_primitive>::value &&
				has_lock_recursive<lock_primitive>::value &&
				has_unlock_recursive<lock_primitive>::value &&
				has_lock_nonrecursive<lock_primitive>::value &&
				has_unlock_nonrecursive<lock_primitive>::value;

			using adapted_policy = typename std::conditional<matches, T_, void>::type;
		};
	}


	template < typename T_ >
	struct policy_concept
	{
		using adapted_policy = typename wigwag::detail::policy_version_detector<detail::check_lock_primitive_v1_0<T_>>::adapted_policy;
	};

#include <wigwag/detail/enable_warnings.hpp>

}}

#endif
