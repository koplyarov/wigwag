#ifndef WIGWAG_POLICIES_LIFE_ASSURANCE_NONE_HPP
#define WIGWAG_POLICIES_LIFE_ASSURANCE_NONE_HPP

// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <wigwag/detail/config.hpp>


namespace wigwag {
namespace life_assurance
{

#include <wigwag/detail/disable_warnings.hpp>

	struct none
	{
		class shared_data
		{ };

		struct life_assurance
		{
			void reset_life_assurance(const shared_data&)
			{ }

			bool node_should_be_released() const
			{ return false; }

			bool release_node() const
			{ return true; }
		};

		struct life_checker
		{
			life_checker(const shared_data&, const life_assurance&) WIGWAG_NOEXCEPT { }
		};

		struct execution_guard
		{
			execution_guard(const life_checker&) WIGWAG_NOEXCEPT { }
			execution_guard(const shared_data&, const life_assurance&) WIGWAG_NOEXCEPT { }
			int is_alive() const WIGWAG_NOEXCEPT { return true; }
		};
	};

#include <wigwag/detail/enable_warnings.hpp>

}}

#endif
