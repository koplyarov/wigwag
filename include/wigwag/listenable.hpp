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
#include <wigwag/detail/policy_picker.hpp>
#include <wigwag/policies.hpp>


namespace wigwag
{

#include <wigwag/detail/disable_warnings.hpp>

	namespace detail
	{
		template <template <typename> class PolicyConcept_> struct listenable_default_policies;
		template <> struct listenable_default_policies<exception_handling::policy_concept> { using type = exception_handling::default_; };
		template <> struct listenable_default_policies<threading::policy_concept> { using type = threading::default_; };
		template <> struct listenable_default_policies<state_populating::policy_concept> { using type = state_populating::default_; };
		template <> struct listenable_default_policies<life_assurance::policy_concept> { using type = life_assurance::default_; };
	}


	template <
			typename ListenerType_,
			typename... Policies_
		>
	class listenable
	{
		template < template <typename> class PolicyConcept_ >
		using policy = typename detail::policy_picker<PolicyConcept_, detail::listenable_default_policies, Policies_...>::type;

		using exception_handling_policy = policy<exception_handling::policy_concept>;
		using threading_policy = policy<threading::policy_concept>;
		using state_populating_policy = policy<state_populating::policy_concept>;
		using life_assurance_policy = policy<life_assurance::policy_concept>;

	public:
		using listener_type = ListenerType_;

	private:
		using impl_type = detail::listenable_impl<ListenerType_, exception_handling_policy, threading_policy, state_populating_policy, life_assurance_policy>;
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

		token connect(const ListenerType_& handler, handler_attributes attributes = handler_attributes::none) const
		{ return _impl->connect(handler, attributes); }

		template < typename InvokeListenerFunc_ >
		void invoke(const InvokeListenerFunc_& invoke_listener_func)
		{ _impl->invoke(invoke_listener_func); }
	};

#include <wigwag/detail/enable_warnings.hpp>

}

#endif
