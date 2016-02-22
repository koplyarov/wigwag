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

			bool node_deleted_on_finalize() const
			{ return false; }

			bool should_be_finalized() const
			{ return false; }

			template < typename HandlerNode_ >
			void release_external_ownership(const HandlerNode_* node)
			{ delete node; }

			template < typename HandlerNode_ >
			void finalize(const HandlerNode_*)
			{ }
		};

		struct life_checker
		{
			life_checker(const shared_data&, const life_assurance&) noexcept { }
		};

		struct execution_guard
		{
			execution_guard(const life_checker&) noexcept { }
			execution_guard(const shared_data&, const life_assurance&) noexcept { }
			int is_alive() const noexcept { return true; }
		};
	};

#include <wigwag/detail/enable_warnings.hpp>

}}

#endif
