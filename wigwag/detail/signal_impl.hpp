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


namespace wigwag {
namespace detail
{

	template <
			typename Signature_,
			typename ExceptionHandlingPolicy_,
			typename ThreadingPolicy_,
			typename StatePopulatingPolicy_,
			typename HandlersStackContainerPolicy_,
			typename LifeAssurancePolicy_
		>
	class signal_impl
		:	public signal_connector_impl<Signature_>,
			public intrusive_ref_counter<signal_impl<Signature_, ExceptionHandlingPolicy_, ThreadingPolicy_, StatePopulatingPolicy_, HandlersStackContainerPolicy_, LifeAssurancePolicy_>>,
			private ExceptionHandlingPolicy_,
			private ThreadingPolicy_::lock_primitive,
			private StatePopulatingPolicy_::template handler_processor<Signature_>
	{
		using ref_counter_base = intrusive_ref_counter<signal_impl<Signature_, ExceptionHandlingPolicy_, ThreadingPolicy_, StatePopulatingPolicy_, HandlersStackContainerPolicy_, LifeAssurancePolicy_>>;

	public:
		using handler_type = std::function<Signature_>;

		using exception_handler = ExceptionHandlingPolicy_;
		using lock_primitive = typename ThreadingPolicy_::lock_primitive;
		using handler_processor = typename StatePopulatingPolicy_::template handler_processor<Signature_>;

		using life_assurance = typename LifeAssurancePolicy_::life_assurance;
		using life_checker = typename LifeAssurancePolicy_::life_checker;

		struct handler_info : private life_checker
		{
			handler_type		handler;

			handler_info(life_checker lc, handler_type h)
				: life_checker(lc), handler(h)
			{ }

			const life_checker& get_life_checker() const { return *this; }
			const handler_type& get_handler() const { return handler; }
		};

		struct handler_node : public token::implementation, private life_assurance, public detail::intrusive_list_node
		{
			intrusive_ptr<signal_impl>		_signal_impl;
			handler_type					_handler;

		public:
			handler_node(life_assurance&& la, const intrusive_ptr<signal_impl>& impl, handler_type handler)
#if !WIGWAG_NOEXCEPTIONS
			try
#endif
				: life_assurance(std::move(la)), _signal_impl(impl), _handler(handler)
			{ _signal_impl->get_handlers_container().push_back(*this); }
#if !WIGWAG_NOEXCEPTIONS
			catch(...)
			{
				life_assurance::release();
				throw;
			}
#endif

			~handler_node()
			{
				_signal_impl->get_lock_primitive().lock_connect();
				auto sg = detail::at_scope_exit([&] { _signal_impl->get_lock_primitive().unlock_connect(); } );

				life_assurance::release();
				_signal_impl->get_handler_processor().withdraw_state(_handler);
				_signal_impl->get_handlers_container().erase(*this);
			}

			operator handler_info() const
			{ return handler_info(life_assurance::get_life_checker(), _handler); }
		};

		using handlers_container = detail::intrusive_list<handler_node>;

	private:
		handlers_container					_handlers;

	public:
		signal_impl() { }

#define DETAIL_WIGWAG_STORAGE_CTOR_ENABLER(...) typename std::enable_if<__VA_ARGS__, enabler>::type e = enabler()

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


		handlers_container& get_handlers_container() { return _handlers; }
		const handlers_container& get_handlers_container() const { return _handlers; }

		const lock_primitive& get_lock_primitive() const { return *this; }
		const exception_handler& get_exception_handler() const { return *this; }
		const handler_processor& get_handler_processor() const { return *this; }

		virtual void add_ref() { ref_counter_base::add_ref(); }
		virtual void release() { ref_counter_base::release(); }

		virtual token connect(const std::function<Signature_>& handler)
		{
			get_lock_primitive().lock_connect();
			auto sg = detail::at_scope_exit([&] { get_lock_primitive().unlock_connect(); } );

			get_exception_handler().handle_exceptions([&] { get_handler_processor().populate_state(handler); });

			add_ref();
			intrusive_ptr<signal_impl> self(this);

			return token::create<handler_node>(life_assurance(), self, handler);
		}

		virtual token connect(const std::shared_ptr<task_executor>& worker, const std::function<Signature_>& handler)
		{
			get_lock_primitive().lock_connect();
			auto sg = detail::at_scope_exit([&] { get_lock_primitive().unlock_connect(); } );

			life_assurance la;
			async_handler<Signature_, LifeAssurancePolicy_> real_handler(worker, la.get_life_checker(), handler);

			get_exception_handler().handle_exceptions([&] { get_handler_processor().populate_state(real_handler); });

			add_ref();
			intrusive_ptr<signal_impl> self(this);

			return token::create<handler_node>(std::move(la), self, real_handler);
		}
	};

}}

#endif
