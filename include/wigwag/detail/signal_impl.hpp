#ifndef WIGWAG_DETAIL_SIGNAL_IMPL_HPP
#define WIGWAG_DETAIL_SIGNAL_IMPL_HPP

// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <wigwag/detail/async_handler.hpp>
#include <wigwag/detail/config.hpp>
#include <wigwag/detail/enabler.hpp>
#include <wigwag/detail/intrusive_list.hpp>
#include <wigwag/detail/intrusive_ptr.hpp>
#include <wigwag/detail/intrusive_ref_counter.hpp>
#include <wigwag/detail/signal_connector_impl.hpp>

#include <iostream>


namespace wigwag {
namespace detail
{

#include <wigwag/detail/disable_warnings.hpp>

	template <
			typename Signature_,
			typename ExceptionHandlingPolicy_,
			typename ThreadingPolicy_,
			typename StatePopulatingPolicy_,
			typename LifeAssurancePolicy_
		>
	class signal_impl
		:	public signal_connector_impl<Signature_>,
			private intrusive_ref_counter<signal_impl<Signature_, ExceptionHandlingPolicy_, ThreadingPolicy_, StatePopulatingPolicy_, LifeAssurancePolicy_>>,
			private LifeAssurancePolicy_::signal_data,
			private ExceptionHandlingPolicy_,
			private ThreadingPolicy_::lock_primitive,
			private StatePopulatingPolicy_::template handler_processor<std::function<Signature_>>
	{
		friend class intrusive_ref_counter<signal_impl<Signature_, ExceptionHandlingPolicy_, ThreadingPolicy_, StatePopulatingPolicy_, LifeAssurancePolicy_>>;
		using ref_counter_base = intrusive_ref_counter<signal_impl<Signature_, ExceptionHandlingPolicy_, ThreadingPolicy_, StatePopulatingPolicy_, LifeAssurancePolicy_>>;

	public:
		using handler_type = std::function<Signature_>;

		using exception_handler = ExceptionHandlingPolicy_;
		using lock_primitive = typename ThreadingPolicy_::lock_primitive;
		using handler_processor = typename StatePopulatingPolicy_::template handler_processor<handler_type>;

		using life_assurance = typename LifeAssurancePolicy_::life_assurance;
		using life_checker = typename LifeAssurancePolicy_::life_checker;
		using execution_guard = typename LifeAssurancePolicy_::execution_guard;

		class handler_node : public token::implementation, private life_assurance, private detail::intrusive_list_node
		{
			friend class detail::intrusive_list<handler_node>;

			union handler_storage
			{
				handler_type obj;

				handler_storage(const handler_type& handler) : obj(handler) { }
				~handler_storage() { }
			};

		private:
			intrusive_ptr<signal_impl>		_signal_impl;
			handler_storage					_handler;

		public:
			template < typename MakeHandlerFunc_ >
			handler_node(const intrusive_ptr<signal_impl>& impl, const MakeHandlerFunc_& mhf)
				: _signal_impl(impl), _handler(mhf(life_checker(*_signal_impl, *this)))
			{ _signal_impl->get_handlers_container().push_back(*this); }

			handler_node(const intrusive_ptr<signal_impl>& impl, handler_type handler)
				: _signal_impl(impl), _handler(handler)
			{ _signal_impl->get_handlers_container().push_back(*this); }

			~handler_node()
			{
				if (life_assurance::node_deleted_on_finalize())
				{
					_signal_impl->get_handlers_container().erase(*this);
				}
				else
				{
					_signal_impl->get_lock_primitive().lock_connect();
					auto sg = detail::at_scope_exit([&] { _signal_impl->get_lock_primitive().unlock_connect(); } );
					_signal_impl->get_handlers_container().erase(*this);
				}
			}

			virtual void release_token_impl()
			{
				life_assurance::reset_life_assurance(*_signal_impl);
				_signal_impl->get_handler_processor().withdraw_state(_signal_impl->get_lock_primitive(), _handler.obj);
				_handler.obj.~handler_type();
				life_assurance::release_external_ownership(this);
			}

			bool should_be_finalized() const
			{ return life_assurance::should_be_finalized(); }

			void finalize_node()
			{ life_assurance::finalize(this); }

			const handler_type& get_handler() const { return _handler.obj; }
			const life_assurance& get_life_assurance() const { return *this; }
		};

		using handlers_container = detail::intrusive_list<handler_node>;

	private:
		handlers_container					_handlers;

	public:
		signal_impl() { }

#define DETAIL_WIGWAG_STORAGE_CTOR_ENABLER(...) typename std::enable_if<__VA_ARGS__, enabler>::type = enabler()

		template < typename T_ > signal_impl(T_ eh, DETAIL_WIGWAG_STORAGE_CTOR_ENABLER(std::is_constructible<exception_handler, T_&&>::value)) : exception_handler(std::move(eh)) { }
		template < typename T_ > signal_impl(T_ lp, DETAIL_WIGWAG_STORAGE_CTOR_ENABLER(std::is_constructible<lock_primitive, T_&&>::value)) : lock_primitive(std::move(lp)) { }
		template < typename T_ > signal_impl(T_ hp, DETAIL_WIGWAG_STORAGE_CTOR_ENABLER(std::is_constructible<handler_processor, T_&&>::value)) : handler_processor(std::move(hp)) { }

		template < typename T_, typename U_ >
		signal_impl(T_ eh, U_ lp, DETAIL_WIGWAG_STORAGE_CTOR_ENABLER(std::is_constructible<exception_handler, T_&&>::value && std::is_constructible<lock_primitive, U_&&>::value))
			: exception_handler(std::move(eh)), lock_primitive(std::move(lp))
		{ }

		template < typename T_, typename U_ >
		signal_impl(T_ eh, U_ hp, DETAIL_WIGWAG_STORAGE_CTOR_ENABLER(std::is_constructible<exception_handler, T_&&>::value && std::is_constructible<handler_processor, U_&&>::value))
			: exception_handler(std::move(eh)), handler_processor(std::move(hp))
		{ }

		template < typename T_, typename U_ >
		signal_impl(T_ lp, U_ hp, DETAIL_WIGWAG_STORAGE_CTOR_ENABLER(std::is_constructible<lock_primitive, T_&&>::value && std::is_constructible<handler_processor, U_&&>::value))
			: lock_primitive(std::move(lp)), handler_processor(std::move(hp))
		{ }

#undef DETAIL_WIGWAG_STORAGE_CTOR_ENABLER

		signal_impl(exception_handler eh, lock_primitive lp, handler_processor hp)
			: exception_handler(std::move(eh)), lock_primitive(std::move(lp)), handler_processor(std::move(hp))
		{ }

		signal_impl(const signal_impl&) = delete;
		signal_impl& operator = (const signal_impl&) = delete;


		void finalize_nodes()
		{
			for (auto it = _handlers.begin(); it != _handlers.end();)
				(it++)->finalize_node();
		}


		handlers_container& get_handlers_container() { return _handlers; }
		const handlers_container& get_handlers_container() const { return _handlers; }

		const lock_primitive& get_lock_primitive() const { return *this; }
		const exception_handler& get_exception_handler() const { return *this; }
		const handler_processor& get_handler_processor() const { return *this; }

		virtual void add_ref() { ref_counter_base::add_ref(); }
		virtual void release() { ref_counter_base::release(); }

		virtual token connect(const handler_type& handler)
		{
			get_lock_primitive().lock_connect();
			auto sg = detail::at_scope_exit([&] { get_lock_primitive().unlock_connect(); } );

			get_exception_handler().handle_exceptions([&] { get_handler_processor().populate_state(handler); });

			add_ref();
			intrusive_ptr<signal_impl> self(this);

			return token::create<handler_node>(self, handler);
		}

		virtual token connect(const std::shared_ptr<task_executor>& worker, const handler_type& handler)
		{
			get_lock_primitive().lock_connect();
			auto sg = detail::at_scope_exit([&] { get_lock_primitive().unlock_connect(); } );

			add_ref();
			intrusive_ptr<signal_impl> self(this);

			return token::create<handler_node>(self,
					[&](const life_checker& lc) {
						async_handler<Signature_, LifeAssurancePolicy_> real_handler(worker, lc, handler);
						get_exception_handler().handle_exceptions([&] { get_handler_processor().populate_state(real_handler); });
						return real_handler;
					});
		}

		template < typename... Args_ >
		void invoke(Args_&&... args)
		{
			get_lock_primitive().lock_invoke();
			auto sg = detail::at_scope_exit([&] { get_lock_primitive().unlock_invoke(); } );
			for (auto it = _handlers.begin(); it != _handlers.end();)
			{
				if (it->should_be_finalized())
				{
					(it++)->finalize_node();
					continue;
				}

				execution_guard g(*this, it->get_life_assurance());
				if (g.is_alive())
					get_exception_handler().handle_exceptions(it->get_handler(), std::forward<Args_>(args)...);
				++it;
			}
		}
	};

#include <wigwag/detail/enable_warnings.hpp>

}}

#endif
