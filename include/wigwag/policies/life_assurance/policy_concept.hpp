#ifndef WIGWAG_POLICIES_LIFE_ASSURANCE_CONCEPT_HPP
#define WIGWAG_POLICIES_LIFE_ASSURANCE_CONCEPT_HPP

// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <wigwag/detail/type_expression_check.hpp>


namespace wigwag {
namespace life_assurance
{

	namespace detail
	{
		WIGWAG_DECLARE_TYPE_EXPRESSION_CHECK(has_shared_data, std::declval<typename T_::shared_data>());
		WIGWAG_DECLARE_TYPE_EXPRESSION_CHECK(has_life_assurance, std::declval<typename T_::life_assurance>());
		WIGWAG_DECLARE_TYPE_EXPRESSION_CHECK(has_life_checker, std::declval<typename T_::life_checker>());
		WIGWAG_DECLARE_TYPE_EXPRESSION_CHECK(has_execution_guard, std::declval<typename T_::execution_guard>());

		//WIGWAG_DECLARE_TYPE_EXPRESSION_CHECK(has_release_life_assurance, std::declval<T_>().release_life_assurance(std::declval<>()));
		WIGWAG_DECLARE_TYPE_EXPRESSION_CHECK(has_node_should_be_released, std::declval<const T_&>().node_should_be_released());
		WIGWAG_DECLARE_TYPE_EXPRESSION_CHECK(has_release_node, std::declval<const T_&>().release_node());

		//WIGWAG_DECLARE_TYPE_EXPRESSION_CHECK(has_shared_data_and_life_assurance_ctor, T_(std::declval<const shared_data&>(), std::declval<const life_assurance&>()));
		//WIGWAG_DECLARE_TYPE_EXPRESSION_CHECK(has_life_checker_ctor, T_(std::declval<const life_checker&>()));

		WIGWAG_DECLARE_TYPE_EXPRESSION_CHECK(has_is_alive, std::declval<const T_&>().is_alive());

		template < typename T_, bool HasLockPrimitive_ =  detail::has_life_assurance<T_>::value >
		struct check_life_assurance
		{ static constexpr bool value = false; };

		template < typename T_ >
		struct check_life_assurance<T_, true>
		{
			using life_assurance = typename T_::life_assurance;
			static constexpr bool value =
				detail::has_node_should_be_released<life_assurance>::value &&
				detail::has_release_node<life_assurance>::value;
		};


		template < typename T_, bool HasLockPrimitive_ =  detail::has_life_checker<T_>::value >
		struct check_life_checker
		{ static constexpr bool value = false; };

		template < typename T_ >
		struct check_life_checker<T_, true>
		{
			using life_checker = typename T_::life_checker;
			static constexpr bool value = true;
		};


		template < typename T_, bool HasLockPrimitive_ =  detail::has_execution_guard<T_>::value >
		struct check_execution_guard
		{ static constexpr bool value = false; };

		template < typename T_ >
		struct check_execution_guard<T_, true>
		{
			using execution_guard = typename T_::execution_guard;
			static constexpr bool value = has_is_alive<execution_guard>::value;
		};

		template < typename T_ >
		struct check_policy_v1_0
		{
			static constexpr bool matches =
				has_shared_data<T_>::value &&
				check_life_assurance<T_>::value &&
				check_life_checker<T_>::value &&
				check_execution_guard<T_>::value;

			using version = typename std::conditional<matches, wigwag::detail::api_version<1, 0>, std::false_type>::type;
		};
	}


	template < typename T_ >
	struct policy_concept
	{
		using version = typename wigwag::detail::policy_version_detector<detail::check_policy_v1_0<T_>>::version;
	};

}}

#endif
