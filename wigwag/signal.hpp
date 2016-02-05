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
#include <wigwag/life_token.hpp>
#include <wigwag/signal_policies.hpp>
#include <wigwag/token.hpp>

#include <list>


namespace wigwag
{

	template <
			typename Signature_,
			typename ExceptionHandlingPolicy_ = exception_handling::rethrow,
			typename ThreadingPolicy_ = threading::own_recursive_mutex,
			typename StatePopulatingPolicy_ = state_populating::populator_only,
			typename HandlersStoragePolicy_ = handlers_container::list,
			typename LifeAssurancePolicy_ = life_assurance::life_tokens,
			typename ImplStoragePolicy_ = impl_storage::shared
		>
	class basic_signal
	{
	private:
		using handler_type = std::function<Signature_>;

		using lock_primitive = typename ThreadingPolicy_::lock_primitive;
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

		using handlers_container =  typename HandlersStoragePolicy_::template handlers_container<handler_info>;
		using handlers_stack_container = typename handlers_container::handlers_stack_container;
		using handler_id = typename handlers_container::handler_id;

		using storage = typename ImplStoragePolicy_::template storage<ExceptionHandlingPolicy_, lock_primitive, handler_processor, handlers_container>;
		using storage_ref = typename ImplStoragePolicy_::template storage_ref<ExceptionHandlingPolicy_, lock_primitive, handler_processor, handlers_container>;

	private:
		struct connection : public token::implementation, private life_assurance
		{
			storage_ref		_storage_ref;
			handler_id		_id;

		public:
			connection(life_assurance&& la, storage_ref sr, handler_id id)
			try : life_assurance(std::move(la)), _storage_ref(sr), _id(id)
			{ static_assert(std::is_nothrow_move_constructible<life_assurance>::value, "ASD"); }
			catch(...)
			{
				life_assurance::release();
				throw;
			}

			~connection()
			{
				_storage_ref.get_lock_primitive().lock_connect();
				auto sg = detail::at_scope_exit([&]() { _storage_ref.get_lock_primitive().unlock_connect(); } );

				life_assurance::release();
				const auto& handler = _storage_ref.get_handlers_container().get_handler_info(_id).handler;
				_storage_ref.get_handler_processor().withdraw_state(handler);
				_storage_ref.get_handlers_container().erase_handler(_id);
			}
		};

	private:
		storage		_storage;

	public:
		basic_signal()
		{ }

		template < typename... Args_ >
		basic_signal(Args_&&... args)
			: _storage(std::forward<Args_>(args)...)
		{ }

		basic_signal(const basic_signal&) = delete;
		basic_signal& operator = (const basic_signal&) = delete;

		token connect(const handler_type& handler)
		{
			_storage.get_lock_primitive().lock_connect();
			auto sg = detail::at_scope_exit([&]() { _storage.get_lock_primitive().unlock_connect(); } );

			handle_exceptions(_storage.get_exception_handler(), [&]() { _storage.get_handler_processor().populate_state(handler); });

			life_assurance la;
			auto id = _storage.get_handlers_container().add_handler(handler_info(la.get_life_checker(), handler));
			return token::create<connection>(std::move(la), storage_ref(_storage), id);
		}

		template < typename... Args >
		void operator() (Args&&... args) const
		{
			handlers_stack_container handlers_copy;

			{
				_storage.get_lock_primitive().lock_invoke();
				auto sg = detail::at_scope_exit([&]() { _storage.get_lock_primitive().unlock_invoke(); } );

				handlers_copy.assign(_storage.get_handlers_container().get_handlers().begin(), _storage.get_handlers_container().get_handlers().end());
			}

			for (const auto& h : handlers_copy)
			{
				execution_guard g(h.get_life_checker());
				if (g.is_alive())
					handle_exceptions(_storage.get_exception_handler(), [&]() { h.get_handler()(std::forward<Args>(args)...); });
			}
		}

	private:
		template < typename Func_, typename... Args_ >
		static void handle_exceptions(const ExceptionHandlingPolicy_& exceptionHandler, Func_&& func)
		{
			try
			{ func(); }
			catch (const std::exception& ex)
			{ exceptionHandler.handle_std_exception(ex); }
			catch (...)
			{ exceptionHandler.handle_unknown_exception(); }
		}
	};


	template < typename Signature_ >
	using signal = basic_signal<Signature_>;

}

#endif
