#ifndef WIGWAG_POLICIES_STATE_POPULATING_POPULATOR_ONLY_HPP
#define WIGWAG_POLICIES_STATE_POPULATING_POPULATOR_ONLY_HPP

// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <wigwag/detail/config.hpp>

#include <functional>


namespace wigwag {
namespace state_populating
{

#include <wigwag/detail/disable_warnings.hpp>

	struct populator_only
	{
		template < typename HandlerType_ >
		class handler_processor
		{
			using handler_processor_func = std::function<void(const HandlerType_&)>;

		private:
			handler_processor_func		_populator;

		public:
			handler_processor(handler_processor_func populator = &populator_only::handler_processor<HandlerType_>::empty_handler)
				: _populator(populator)
			{ }

			void populate_state(const HandlerType_& handler) const { _populator(handler); }

			template < typename LockPrimitive_ >
			void withdraw_state(LockPrimitive_&, const HandlerType_&) const WIGWAG_NOEXCEPT { }

			static void empty_handler(const HandlerType_&) WIGWAG_NOEXCEPT { }
		};
	};

#include <wigwag/detail/enable_warnings.hpp>

}}

#endif
