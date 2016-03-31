#ifndef WIGWAG_POLICIES_STATE_POPULATING_POLICY_CONCEPT_HPP
#define WIGWAG_POLICIES_STATE_POPULATING_POLICY_CONCEPT_HPP

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

#include <functional>
#include <mutex>


namespace wigwag {
namespace state_populating
{

	namespace detail
	{
		WIGWAG_DECLARE_TYPE_EXPRESSION_CHECK(has_handler_processor, std::declval<typename T_::template handler_processor<std::function<void()>>>());
		WIGWAG_DECLARE_TYPE_EXPRESSION_CHECK(has_populate_state, std::declval<T_>().populate_state(std::function<void()>()));
		WIGWAG_DECLARE_TYPE_EXPRESSION_CHECK(has_withdraw_state, std::declval<T_>().withdraw_state(std::declval<std::mutex&>(), std::function<void()>()));

		template < typename T_, bool HasLockPrimitive_ =  detail::has_handler_processor<T_>::value >
		struct check_handler_processor_v1_0
		{ using adapted_policy = void; };

		template < typename T_ >
		struct check_handler_processor_v1_0<T_, true>
		{
			using handler_processor = typename T_::template handler_processor<std::function<void()>>;
			static constexpr bool matches =
				has_populate_state<handler_processor>::value &&
				has_withdraw_state<handler_processor>::value;

			using adapted_policy = typename std::conditional<matches, T_, void>::type;
		};
	}


	template < typename T_ >
	struct policy_concept
	{ using adapted_policy = typename wigwag::detail::policy_version_detector<detail::check_handler_processor_v1_0<T_>>::adapted_policy; };

}}

#endif
