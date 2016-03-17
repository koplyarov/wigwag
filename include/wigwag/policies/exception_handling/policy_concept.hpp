#ifndef WIGWAG_POLICIES_EXCEPTION_HANDLING_POLICY_CONCEPT_HPP
#define WIGWAG_POLICIES_EXCEPTION_HANDLING_POLICY_CONCEPT_HPP

// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <wigwag/detail/type_expression_check.hpp>

#include <functional>


namespace wigwag {
namespace exception_handling
{

	namespace detail
	{
		WIGWAG_DECLARE_TYPE_EXPRESSION_CHECK(check_handle_exceptions, std::declval<T_>().handle_exceptions(std::function<void(int, double)>(), 42, 3.14));
	}


	template < typename T_ >
	struct policy_concept
	{
		static constexpr bool matches = detail::check_handle_exceptions<T_>::value;
	};

}}

#endif
