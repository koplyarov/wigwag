#ifndef WIGWAG_SIGNAL_HPP
#define WIGWAG_SIGNAL_HPP

// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <wigwag/detail/at_scope_exit.hpp>
#include <wigwag/detail/signal_impl.hpp>
#include <wigwag/signal_connector.hpp>
#include <wigwag/signal_policies.hpp>
#include <wigwag/token.hpp>


namespace wigwag
{

#include <wigwag/detail/disable_warnings.hpp>

	template <
			typename Signature_,
			typename ExceptionHandlingPolicy_ = exception_handling::default_,
			typename ThreadingPolicy_ = threading::default_,
			typename StatePopulatingPolicy_ = state_populating::default_,
			typename LifeAssurancePolicy_ = life_assurance::default_
		>
	class signal
	{
	private:
		using handler_type = std::function<Signature_>;

		using impl_type = detail::signal_impl<Signature_, ExceptionHandlingPolicy_, ThreadingPolicy_, StatePopulatingPolicy_, LifeAssurancePolicy_>;
		using impl_type_ptr = detail::intrusive_ptr<impl_type>;

	private:
		impl_type_ptr		_impl;

	public:
		template < typename... Args_ >
		signal(Args_&&... args)
			: _impl(new impl_type(std::forward<Args_>(args)...))
		{ }

		~signal()
		{ _impl->finalize_nodes(); }

		signal(const signal&) = delete;
		signal& operator = (const signal&) = delete;

		auto lock_primitive() const -> decltype(_impl->get_lock_primitive().get_primitive())
		{ return _impl->get_lock_primitive().get_primitive(); }

		signal_connector<Signature_> connector() const
		{ return signal_connector<Signature_>(_impl); }

		token connect(const handler_type& handler)
		{ return _impl->connect(handler); }

		token connect(const std::shared_ptr<task_executor>& worker, const std::function<Signature_>& handler)
		{ return _impl->connect(worker, handler); }

		template < typename... Args_ >
		void operator() (Args_&&... args) const
		{ _impl->invoke(std::forward<Args_>(args)...); }
	};

#include <wigwag/detail/enable_warnings.hpp>

}

#endif
