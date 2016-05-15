#ifndef WIGWAG_POLICIES_CREATION_POLICY_CONCEPT_HPP
#define WIGWAG_POLICIES_CREATION_POLICY_CONCEPT_HPP

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
namespace creation
{

	namespace detail
	{
		WIGWAG_DECLARE_TYPE_EXPRESSION_CHECK(has_storage, std::declval<typename T_::template storage<std::shared_ptr<int>, int>>());
		WIGWAG_DECLARE_TYPE_EXPRESSION_CHECK(has_get_ptr, std::declval<const T_>().get_ptr());

#if defined(_MSC_VER) && _MSC_VER < 1900
		WIGWAG_DECLARE_TYPE_EXPRESSION_CHECK(has_create, std::declval<T_>().create<int>(42));
#else
		WIGWAG_DECLARE_TYPE_EXPRESSION_CHECK(has_create, std::declval<T_>().template create<int>(42));
#endif

		template < typename T_, bool HasStorage_ =  detail::has_storage<T_>::value >
		struct check_policy_v1_1
		{ using adapted_policy = void; };

		template < typename T_ >
		struct check_policy_v1_1<T_, true>
		{
			using storage = typename T_::template storage<std::shared_ptr<int>, int>;
			static constexpr bool matches =
				detail::has_create<storage>::value &&
				detail::has_get_ptr<storage>::value;

			using adapted_policy = typename std::conditional<matches, T_, void>::type;
		};
	}


	template < typename T_ >
	struct policy_concept
	{
		using adapted_policy = typename wigwag::detail::policy_version_detector<detail::check_policy_v1_1<T_>>::adapted_policy;
	};

}}

#endif
