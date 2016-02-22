#ifndef WIGWAG_DETAIL_LISTENABLE_IMPL_HPP
#define WIGWAG_DETAIL_LISTENABLE_IMPL_HPP

// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <wigwag/detail/config.hpp>
#include <wigwag/detail/enabler.hpp>
#include <wigwag/detail/intrusive_list.hpp>
#include <wigwag/detail/intrusive_ptr.hpp>
#include <wigwag/detail/intrusive_ref_counter.hpp>
#include <wigwag/token.hpp>


namespace wigwag {
namespace detail
{

#include <wigwag/detail/disable_warnings.hpp>

	template <
			typename HandlerType_,
			typename ExceptionHandlingPolicy_,
			typename ThreadingPolicy_,
			typename StatePopulatingPolicy_,
			typename LifeAssurancePolicy_
		>
	class listenable_impl
		:	private intrusive_ref_counter<listenable_impl<HandlerType_, ExceptionHandlingPolicy_, ThreadingPolicy_, StatePopulatingPolicy_, LifeAssurancePolicy_>>,
			private LifeAssurancePolicy_::signal_data,
			private ExceptionHandlingPolicy_,
			private ThreadingPolicy_::lock_primitive,
			private StatePopulatingPolicy_::template handler_processor<HandlerType_>
	{
		friend class intrusive_ref_counter<listenable_impl<HandlerType_, ExceptionHandlingPolicy_, ThreadingPolicy_, StatePopulatingPolicy_, LifeAssurancePolicy_>>;
		using ref_counter_base = intrusive_ref_counter<listenable_impl<HandlerType_, ExceptionHandlingPolicy_, ThreadingPolicy_, StatePopulatingPolicy_, LifeAssurancePolicy_>>;

	public:
		using handler_type = HandlerType_;

		using exception_handler = ExceptionHandlingPolicy_;
		using lock_primitive = typename ThreadingPolicy_::lock_primitive;
		using handler_processor = typename StatePopulatingPolicy_::template handler_processor<handler_type>;

		using life_assurance = typename LifeAssurancePolicy_::life_assurance;
		using life_checker = typename LifeAssurancePolicy_::life_checker;
		using execution_guard = typename LifeAssurancePolicy_::execution_guard;

	protected:
		class handler_node : public token::implementation, private life_assurance, private detail::intrusive_list_node
		{
			friend class detail::intrusive_list<handler_node>;

			union handler_storage
			{
				handler_type obj;

				handler_storage(const handler_type& handler) : obj(handler) { }
				~handler_storage() { }
			};

			class lock_primitive_adapter
			{
			private:
				const lock_primitive&	_lp;

			public:
				lock_primitive_adapter(const lock_primitive& lp)
					: _lp(lp)
				{ }

				void lock() const { _lp.lock_nonrecursive(); }
				void unlock() const { _lp.unlock_nonrecursive(); }
			};

		private:
			intrusive_ptr<listenable_impl>	_listenable_impl;
			handler_storage					_handler;

		public:
			template < typename MakeHandlerFunc_ >
			handler_node(const intrusive_ptr<listenable_impl>& impl, const MakeHandlerFunc_& mhf)
				: _listenable_impl(impl), _handler(mhf(life_checker(*_listenable_impl, *this)))
			{ _listenable_impl->get_handlers_container().push_back(*this); }

			handler_node(const intrusive_ptr<listenable_impl>& impl, handler_type handler)
				: _listenable_impl(impl), _handler(handler)
			{ _listenable_impl->get_handlers_container().push_back(*this); }

			~handler_node()
			{
				if (life_assurance::node_deleted_on_finalize())
				{
					_listenable_impl->get_handlers_container().erase(*this);
				}
				else
				{
					_listenable_impl->get_lock_primitive().lock_nonrecursive();
					auto sg = detail::at_scope_exit([&] { _listenable_impl->get_lock_primitive().unlock_nonrecursive(); } );
					_listenable_impl->get_handlers_container().erase(*this);
				}
			}

			virtual void release_token_impl()
			{
				life_assurance::reset_life_assurance(*_listenable_impl);
				lock_primitive_adapter lp(_listenable_impl->get_lock_primitive());
				_listenable_impl->get_handler_processor().withdraw_state(lp, _handler.obj);
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

		handlers_container					_handlers;

	public:
		listenable_impl() { }
		virtual ~listenable_impl() { }

#define DETAIL_LISTENABLE_IMPL_CTOR_ENABLER(...) typename std::enable_if<__VA_ARGS__, enabler>::type = enabler()

		template < typename T_ > listenable_impl(T_ eh, DETAIL_LISTENABLE_IMPL_CTOR_ENABLER(std::is_constructible<exception_handler, T_&&>::value)) : exception_handler(std::move(eh)) { }
		template < typename T_ > listenable_impl(T_ lp, DETAIL_LISTENABLE_IMPL_CTOR_ENABLER(std::is_constructible<lock_primitive, T_&&>::value)) : lock_primitive(std::move(lp)) { }
		template < typename T_ > listenable_impl(T_ hp, DETAIL_LISTENABLE_IMPL_CTOR_ENABLER(std::is_constructible<handler_processor, T_&&>::value)) : handler_processor(std::move(hp)) { }

		template < typename T_, typename U_ >
		listenable_impl(T_ eh, U_ lp, DETAIL_LISTENABLE_IMPL_CTOR_ENABLER(std::is_constructible<exception_handler, T_&&>::value && std::is_constructible<lock_primitive, U_&&>::value))
			: exception_handler(std::move(eh)), lock_primitive(std::move(lp))
		{ }

		template < typename T_, typename U_ >
		listenable_impl(T_ eh, U_ hp, DETAIL_LISTENABLE_IMPL_CTOR_ENABLER(std::is_constructible<exception_handler, T_&&>::value && std::is_constructible<handler_processor, U_&&>::value))
			: exception_handler(std::move(eh)), handler_processor(std::move(hp))
		{ }

		template < typename T_, typename U_ >
		listenable_impl(T_ lp, U_ hp, DETAIL_LISTENABLE_IMPL_CTOR_ENABLER(std::is_constructible<lock_primitive, T_&&>::value && std::is_constructible<handler_processor, U_&&>::value))
			: lock_primitive(std::move(lp)), handler_processor(std::move(hp))
		{ }

#undef DETAIL_LISTENABLE_IMPL_CTOR_ENABLER

		listenable_impl(exception_handler eh, lock_primitive lp, handler_processor hp)
			: exception_handler(std::move(eh)), lock_primitive(std::move(lp)), handler_processor(std::move(hp))
		{ }

		listenable_impl(const listenable_impl&) = delete;
		listenable_impl& operator = (const listenable_impl&) = delete;


		void finalize_nodes()
		{
			for (auto it = _handlers.begin(); it != _handlers.end();)
				(it++)->finalize_node();
		}

		void add_ref() { ref_counter_base::add_ref(); }
		void release() { ref_counter_base::release(); }

		token connect(const handler_type& handler)
		{
			get_lock_primitive().lock_nonrecursive();
			auto sg = detail::at_scope_exit([&] { get_lock_primitive().unlock_nonrecursive(); } );

			get_exception_handler().handle_exceptions([&] { get_handler_processor().populate_state(handler); });

			add_ref();
			intrusive_ptr<listenable_impl> self(this);

			return token::create<handler_node>(self, handler);
		}

		template < typename InvokeListenerFunc_ >
		void invoke(const InvokeListenerFunc_& invoke_listener_func)
		{
			get_lock_primitive().lock_recursive();
			auto sg = detail::at_scope_exit([&] { get_lock_primitive().unlock_recursive(); } );
			for (auto it = _handlers.begin(); it != _handlers.end();)
			{
				if (it->should_be_finalized())
				{
					(it++)->finalize_node();
					continue;
				}

				execution_guard g(*this, it->get_life_assurance());
				if (g.is_alive())
					get_exception_handler().handle_exceptions(invoke_listener_func, it->get_handler());
				++it;
			}
		}

		const lock_primitive& get_lock_primitive() const { return *this; }

	protected:
		const typename LifeAssurancePolicy_::signal_data& get_signal_data() const { return *this; }

		handlers_container& get_handlers_container() { return _handlers; }
		const handlers_container& get_handlers_container() const { return _handlers; }

		const exception_handler& get_exception_handler() const { return *this; }
		const handler_processor& get_handler_processor() const { return *this; }
	};

#include <wigwag/detail/enable_warnings.hpp>

}}

#endif
