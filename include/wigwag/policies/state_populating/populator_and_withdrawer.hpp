#ifndef WIGWAG_POLICIES_STATE_POPULATING_POPULATOR_AND_WITHDRAWER_HPP
#define WIGWAG_POLICIES_STATE_POPULATING_POPULATOR_AND_WITHDRAWER_HPP

// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <wigwag/detail/config.hpp>
#include <wigwag/policies/state_populating/tag.hpp>

#include <functional>
#include <mutex>


namespace wigwag {
namespace state_populating
{

#include <wigwag/detail/disable_warnings.hpp>

	struct populator_and_withdrawer
	{
		using tag = state_populating::tag<api_version<2, 0>>;

		template < typename HandlerType_ >
		class handler_processor
		{
			using handler_processor_func = std::function<void(const HandlerType_&)>;

		private:
			handler_processor_func		_populator;
			handler_processor_func		_withdrawer;

		public:
			handler_processor(handler_processor_func populator = handler_processor_func(), handler_processor_func withdrawer = handler_processor_func())
				: _populator(populator), _withdrawer(withdrawer)
			{ }

			template < typename K_, typename V_ >
			handler_processor(const std::pair<K_, V_>& populator_and_withdrawer_pair, typename std::enable_if<std::is_constructible<handler_processor_func, K_>::value && std::is_constructible<handler_processor_func, V_>::value, wigwag::detail::enabler>::type e = wigwag::detail::enabler())
				: _populator(populator_and_withdrawer_pair.first), _withdrawer(populator_and_withdrawer_pair.second)
			{ }

			bool has_populate_state() const WIGWAG_NOEXCEPT { return (bool)_populator; }
			void populate_state(const HandlerType_& handler) const { _populator(handler); }

			bool has_withdraw_state() const WIGWAG_NOEXCEPT { return (bool)_withdrawer; }
			void withdraw_state(const HandlerType_& handler) const { _withdrawer(handler); }
		};
	};

#include <wigwag/detail/enable_warnings.hpp>

}}

#endif
