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


#include <wigwag/detail/creation_storage_adapter.hpp>
#include <wigwag/detail/policies_concepts.hpp>
#include <wigwag/detail/policy_picker.hpp>
#include <wigwag/detail/signal_impl.hpp>
#include <wigwag/policies.hpp>
#include <wigwag/signal_connector.hpp>


namespace wigwag
{

#include <wigwag/detail/disable_warnings.hpp>

	namespace detail
	{
		using signal_policies_config = policies_config<
				policies_config_entry<exception_handling::policy_concept, wigwag::exception_handling::default_>,
				policies_config_entry<threading::policy_concept, wigwag::threading::default_>,
				policies_config_entry<state_populating::policy_concept, wigwag::state_populating::default_>,
				policies_config_entry<life_assurance::policy_concept, wigwag::life_assurance::default_>,
				policies_config_entry<creation::policy_concept, wigwag::creation::default_>,
				policies_config_entry<ref_counter::policy_concept, wigwag::ref_counter::default_>
			>;
	}


	template <
			typename Signature_,
			typename... Policies_
		>
	class signal;

	template <
			typename RetType_,
			typename... ArgTypes_,
			typename... Policies_
		>
	class signal<RetType_(ArgTypes_...), Policies_...>
	{
		using signature = RetType_(ArgTypes_...);

		template < template <typename> class PolicyConcept_ >
		using policy = typename detail::policy_picker<PolicyConcept_, detail::signal_policies_config, Policies_...>::type;

		using exception_handling_policy = policy<detail::exception_handling::policy_concept>;
		using threading_policy = policy<detail::threading::policy_concept>;
		using state_populating_policy = policy<detail::state_populating::policy_concept>;
		using life_assurance_policy = policy<detail::life_assurance::policy_concept>;
		using creation_policy = policy<detail::creation::policy_concept>;
		using ref_counter_policy = policy<detail::ref_counter::policy_concept>;

	public:
		using handler_type = std::function<signature>;

	private:
		using impl_type = detail::signal_impl<signature, exception_handling_policy, threading_policy, state_populating_policy, life_assurance_policy, ref_counter_policy>;
		using impl_type_with_attr = detail::signal_with_attributes_impl<signature, exception_handling_policy, threading_policy, state_populating_policy, life_assurance_policy, ref_counter_policy>;
		using impl_type_ptr = detail::intrusive_ptr<impl_type>;

		using storage = typename creation_policy::template storage<impl_type_ptr, impl_type>;

	private:
		detail::creation_storage_adapter<storage>		_impl;

	public:
		template < typename... Args_ >
		signal(signal_attributes attributes, Args_&&... args)
		{
			if (attributes == signal_attributes::none)
				_impl.template create<impl_type>(std::forward<Args_>(args)...);
			else
				_impl.template create<impl_type_with_attr>(attributes, std::forward<Args_>(args)...);
		}

		template < typename... Args_ >
		signal(Args_&&... args)
		{ _impl.template create<impl_type>(std::forward<Args_>(args)...); }

		template < bool has_default_ctor = std::is_constructible<impl_type>::value, typename = typename std::enable_if<has_default_ctor>::type>
		signal()
		{ _impl.template create<impl_type>(); }

		~signal()
		{
			if (_impl)
			{
				_impl->get_lock_primitive().lock_nonrecursive();
				auto sg = detail::at_scope_exit([&] { _impl->get_lock_primitive().unlock_nonrecursive(); } );

				_impl->finalize_nodes();
			}
		}

		signal(const signal&) = delete;
		signal& operator = (const signal&) = delete;

		auto lock_primitive() const -> decltype(_impl->get_lock_primitive().get_primitive())
		{ return _impl->get_lock_primitive().get_primitive(); }

		signal_connector<signature> connector() const
		{ return signal_connector<signature>(_impl.get_ptr()); }

		template < typename HandlerFunc_ >
		token connect(HandlerFunc_ handler, handler_attributes attributes = handler_attributes::none) const
		{ return _impl->connect(std::move(handler), attributes); }

		template < typename HandlerFunc_ >
		token connect(const std::shared_ptr<task_executor>& worker, HandlerFunc_ handler, handler_attributes attributes = handler_attributes::none) const
		{ return _impl->connect(worker, std::move(handler), attributes); }

		void operator() (ArgTypes_... args) const
		{
			if (_impl)
				_impl->invoke(args...);
		}

	private:
		//template < bool has_default_ctor = std::is_constructible<impl_type>::value>
		//void ensure_impl_created(typename std::enable_if<has_default_ctor, detail::enabler>::type = detail::enabler()) const
		//{
			//if (!_impl)
				//_impl.reset(creation_policy::template create_just_in_time<impl_type>());
		//}

		//template < bool has_default_ctor = std::is_constructible<impl_type>::value>
		//void ensure_impl_created(typename std::enable_if<!has_default_ctor, detail::enabler>::type = detail::enabler()) const
		//{ WIGWAG_ASSERT(_impl, "Internal wigwag error, _impl must have been initialized before!"); }
	};

#include <wigwag/detail/enable_warnings.hpp>

}

#endif
