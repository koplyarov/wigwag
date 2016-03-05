#ifndef WIGWAG_POLICIES_LIFE_ASSURANCE_LIFE_TOKENS_HPP
#define WIGWAG_POLICIES_LIFE_ASSURANCE_LIFE_TOKENS_HPP

// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <wigwag/detail/annotations.hpp>
#include <wigwag/detail/config.hpp>
#include <wigwag/life_token.hpp>

#include <atomic>


namespace wigwag {
namespace life_assurance
{

#include <wigwag/detail/disable_warnings.hpp>

	struct life_tokens
	{
		class life_checker;
		class execution_guard;


		class shared_data
		{ };


		class life_assurance
		{
			friend class life_checker;
			friend class execution_guard;

			life_token					_token;
			mutable std::atomic<int>	_ref_count { 2 };

		public:
			void release_life_assurance(const shared_data&)
			{ _token.release(); }

			bool node_should_be_released() const
			{ return _ref_count == 1; }

			bool release_node() const
			{
				if (--_ref_count == 0)
				{
					WIGWAG_ANNOTATE_HAPPENS_AFTER(this);
					WIGWAG_ANNOTATE_RELEASE(this);

					return true;
				}
				else
				{
					WIGWAG_ANNOTATE_HAPPENS_BEFORE(this);
					return false;
				}
			}
		};


		class life_checker
		{
			friend class execution_guard;

			life_token::checker		_checker;

		public:
			life_checker(const shared_data&, const life_assurance& la) WIGWAG_NOEXCEPT : _checker(la._token) { }
		};


		class execution_guard
		{
			life_token::execution_guard		_guard;

		public:
			execution_guard(const life_checker& c) : _guard(c._checker) { } // TODO: looks like WIGWAG_NOEXCEPT here makes the code faster, check it on other machines
			execution_guard(const shared_data&, const life_assurance& la) : _guard(la._token) { }
			int is_alive() const WIGWAG_NOEXCEPT { return _guard.is_alive(); }
		};
	};

#include <wigwag/detail/enable_warnings.hpp>

}}

#endif
