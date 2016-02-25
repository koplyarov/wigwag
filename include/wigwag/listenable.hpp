#ifndef WIGWAG_LISTENABLE_HPP
#define WIGWAG_LISTENABLE_HPP

// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <wigwag/detail/listenable_impl.hpp>
#include <wigwag/policies.hpp>


namespace wigwag
{

#include <wigwag/detail/disable_warnings.hpp>

	template <
			typename ListenerType_,
			typename ExceptionHandlingPolicy_ = exception_handling::default_,
			typename ThreadingPolicy_ = threading::default_,
			typename StatePopulatingPolicy_ = state_populating::default_,
			typename LifeAssurancePolicy_ = life_assurance::default_
		>
	class listenable
	{
	private:
		using impl_type = detail::listenable_impl<ListenerType_, ExceptionHandlingPolicy_, ThreadingPolicy_, StatePopulatingPolicy_, LifeAssurancePolicy_>;
		using impl_type_ptr = detail::intrusive_ptr<impl_type>;

	private:
		impl_type_ptr		_impl;

	public:
		template < typename... Args_ >
		listenable(Args_&&... args)
			: _impl(new impl_type(std::forward<Args_>(args)...))
		{ }

		~listenable()
		{
			_impl->get_lock_primitive().lock_nonrecursive();
			auto sg = detail::at_scope_exit([&] { _impl->get_lock_primitive().unlock_nonrecursive(); } );

			_impl->finalize_nodes();
		}

		listenable(const listenable&) = delete;
		listenable& operator = (const listenable&) = delete;

		auto lock_primitive() const -> decltype(_impl->get_lock_primitive().get_primitive())
		{ return _impl->get_lock_primitive().get_primitive(); }

		token connect(const ListenerType_& handler, handler_attributes attributes = handler_attributes::none)
		{ return _impl->connect(handler, attributes); }

		template < typename InvokeListenerFunc_ >
		void invoke(const InvokeListenerFunc_& invoke_listener_func)
		{ _impl->invoke(invoke_listener_func); }
	};

#include <wigwag/detail/enable_warnings.hpp>

}

#endif
