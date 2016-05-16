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


#include <wigwag/detail/creation_storage_adapter.hpp>
#include <wigwag/detail/listenable_impl.hpp>
#include <wigwag/detail/policies_concepts.hpp>
#include <wigwag/detail/policy_picker.hpp>
#include <wigwag/policies.hpp>


namespace wigwag
{

#include <wigwag/detail/disable_warnings.hpp>

	namespace detail
	{
		using listenable_policies_config = policies_config<
				policies_config_entry<exception_handling::policy_concept, wigwag::exception_handling::default_>,
				policies_config_entry<threading::policy_concept, wigwag::threading::default_>,
				policies_config_entry<state_populating::policy_concept, wigwag::state_populating::default_>,
				policies_config_entry<life_assurance::policy_concept, wigwag::life_assurance::default_>,
				policies_config_entry<creation::policy_concept, wigwag::creation::default_>,
				policies_config_entry<ref_counter::policy_concept, wigwag::ref_counter::default_>
			>;
	}


	template <
			typename ListenerType_,
			typename... Policies_
		>
	class listenable :
		private detail::policy_picker<detail::creation::policy_concept, detail::listenable_policies_config, Policies_...>::type
	{
		template < template <typename> class PolicyConcept_ >
		using policy = typename detail::policy_picker<PolicyConcept_, detail::listenable_policies_config, Policies_...>::type;

		using exception_handling_policy = policy<detail::exception_handling::policy_concept>;
		using threading_policy = policy<detail::threading::policy_concept>;
		using state_populating_policy = policy<detail::state_populating::policy_concept>;
		using life_assurance_policy = policy<detail::life_assurance::policy_concept>;
		using creation_policy = policy<detail::creation::policy_concept>;
		using ref_counter_policy = policy<detail::ref_counter::policy_concept>;

	public:
		using listener_type = ListenerType_;

	private:
		using impl_type = detail::listenable_impl<ListenerType_, exception_handling_policy, threading_policy, state_populating_policy, life_assurance_policy, ref_counter_policy>;
		using impl_type_ptr = detail::intrusive_ptr<impl_type>;

		using storage = typename creation_policy::template storage<impl_type_ptr, impl_type>;

	private:
		detail::creation_storage_adapter<storage>		_impl;

	public:
		template < bool has_default_ctor = std::is_constructible<impl_type>::value, typename = typename std::enable_if<has_default_ctor>::type>
		listenable()
		{ _impl.template create<impl_type>(); }

		template < typename Arg0_, typename... Args_ >
		listenable(Arg0_&& arg0, Args_&&... args)
		{ _impl.template create<impl_type>(std::forward<Arg0_>(arg0), std::forward<Args_>(args)...); }

		~listenable()
		{
			if (_impl)
			{
				_impl->get_lock_primitive().lock_nonrecursive();
				auto sg = detail::at_scope_exit([&] { _impl->get_lock_primitive().unlock_nonrecursive(); } );

				_impl->finalize_nodes();
			}
		}

		listenable(const listenable&) = delete;
		listenable& operator = (const listenable&) = delete;

		auto lock_primitive() const -> decltype(_impl->get_lock_primitive().get_primitive())
		{ return _impl->get_lock_primitive().get_primitive(); }

		token connect(const ListenerType_& handler, handler_attributes attributes = handler_attributes::none) const
		{ return _impl->connect(handler, attributes); }

		template < typename InvokeListenerFunc_ >
		void invoke(const InvokeListenerFunc_& invoke_listener_func)
		{
			if (_impl)
				_impl->invoke(invoke_listener_func);
		}
	};

#include <wigwag/detail/enable_warnings.hpp>

}

#endif
