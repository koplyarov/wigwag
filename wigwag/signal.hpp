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
#include <wigwag/detail/intrusive_list.hpp>
#include <wigwag/life_token.hpp>
#include <wigwag/signal_policies.hpp>
#include <wigwag/token.hpp>

#include <list>


namespace wigwag
{

	template <
			typename Signature_,
			typename ExceptionHandlingPolicy_ = exception_handling::default_,
			typename ThreadingPolicy_ = threading::default_,
			typename StatePopulatingPolicy_ = state_populating::default_,
			typename HandlersStackContainerPolicy_ = handlers_stack_container::default_,
			typename LifeAssurancePolicy_ = life_assurance::default_,
			typename ImplStoragePolicy_ = impl_storage::default_
		>
	class signal
	{
	private:
		using handler_type = std::function<Signature_>;

		using lock_primitive_type = typename ThreadingPolicy_::lock_primitive;
		using exception_handler = ExceptionHandlingPolicy_;
		using handler_processor = typename StatePopulatingPolicy_::template handler_processor<Signature_>;

		using life_assurance = typename LifeAssurancePolicy_::life_assurance;
		using life_checker = typename LifeAssurancePolicy_::life_checker;
		using execution_guard = typename LifeAssurancePolicy_::execution_guard;

		struct handler_info : private life_checker
		{
			handler_type		handler;

			handler_info(life_checker lc, handler_type h)
				: life_checker(lc), handler(h)
			{ }

			const life_checker& get_life_checker() const { return *this; }
			const handler_type& get_handler() const { return handler; }
		};

		using handlers_stack_container = typename HandlersStackContainerPolicy_::template handlers_stack_container<handler_info>;

		struct handler_node;
		using handlers_container = detail::intrusive_list<handler_node>;

		using storage = typename ImplStoragePolicy_::template storage<ExceptionHandlingPolicy_, lock_primitive_type, handler_processor, handlers_container>;
		using storage_ref = typename ImplStoragePolicy_::template storage_ref<ExceptionHandlingPolicy_, lock_primitive_type, handler_processor, handlers_container>;

		struct handler_node : public token::implementation, private life_assurance, public detail::intrusive_list_node
		{
			storage_ref		_storage_ref;
			handler_type	_handler;

		public:
			handler_node(storage_ref sr, handler_type handler)
			try : _storage_ref(sr), _handler(handler)
			{
				static_assert(std::is_nothrow_move_constructible<life_assurance>::value, "life_assurance object should have a noexcept move constructor!");
				_storage_ref.get_handlers_container().push_back(*this);
			}
			catch(...)
			{
				life_assurance::release();
				throw;
			}

			~handler_node()
			{
				_storage_ref.get_lock_primitive().lock_connect();
				auto sg = detail::at_scope_exit([&] { _storage_ref.get_lock_primitive().unlock_connect(); } );

				life_assurance::release();
				_storage_ref.get_handler_processor().withdraw_state(_handler);
				_storage_ref.get_handlers_container().erase(*this);
			}

			operator handler_info() const
			{ return handler_info(life_assurance::get_life_checker(), _handler); }
		};

	private:
		storage		_storage;

	public:
		signal()
		{ }

		template < typename... Args_ >
		signal(Args_&&... args)
			: _storage(std::forward<Args_>(args)...)
		{ }

		signal(const signal&) = delete;
		signal& operator = (const signal&) = delete;

		auto lock_primitive() const -> decltype(_storage.get_lock_primitive().get_primitive())
		{ return _storage.get_lock_primitive().get_primitive(); }

		token connect(const handler_type& handler)
		{
			_storage.get_lock_primitive().lock_connect();
			auto sg = detail::at_scope_exit([&] { _storage.get_lock_primitive().unlock_connect(); } );

			handle_exceptions(_storage.get_exception_handler(), [&] { _storage.get_handler_processor().populate_state(handler); });

			return token::create<handler_node>(storage_ref(_storage), handler);
		}

		template < typename... Args_ >
		void operator() (Args_&&... args) const
		{
			handlers_stack_container handlers_copy;

			{
				_storage.get_lock_primitive().lock_invoke();
				auto sg = detail::at_scope_exit([&] { _storage.get_lock_primitive().unlock_invoke(); } );

				handlers_copy.assign(_storage.get_handlers_container().begin(), _storage.get_handlers_container().end());
			}

			for (const auto& h : handlers_copy)
			{
				execution_guard g(h.get_life_checker());
				if (g.is_alive())
					handle_exceptions(_storage.get_exception_handler(), h.get_handler(), std::forward<Args_>(args)...);
			}
		}

	private:
		template < typename Func_, typename... Args_ >
		static void handle_exceptions(const ExceptionHandlingPolicy_& exceptionHandler, Func_&& func, Args_&&... args)
		{
			try
			{ func(std::forward<Args_>(args)...); }
			catch (const std::exception& ex)
			{ exceptionHandler.handle_std_exception(ex); }
			catch (...)
			{ exceptionHandler.handle_unknown_exception(); }
		}
	};


}

#endif
