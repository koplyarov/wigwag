#ifndef WIGWAG_SIGNAL_POLICIES_HPP
#define WIGWAG_SIGNAL_POLICIES_HPP

// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <wigwag/detail/enabler.hpp>
#include <wigwag/life_token.hpp>

#include <list>
#include <memory>
#include <mutex>
#include <vector>

#include <stdio.h>


namespace wigwag
{

	namespace exception_handling
	{
		struct rethrow
		{
			void handle_std_exception(const std::exception& ex) const { throw; }
			void handle_unknown_exception() const { throw; }
		};

		struct print_to_stderr
		{
			void handle_std_exception(const std::exception& ex) const { fprintf(stderr, "std::exception in signal handler: %s\n", ex.what()); }
			void handle_unknown_exception() const { fprintf(stderr, "Unknown exception in signal handler!\n"); }
		};
	}


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	namespace threading
	{
		struct own_recursive_mutex
		{
			class lock_primitive
			{
			private:
				mutable std::recursive_mutex	_mutex;

			public:
				std::recursive_mutex& get_primitive() const { return _mutex; }

				void lock_connect() const { _mutex.lock(); }
				void unlock_connect() const { _mutex.unlock(); }

				void lock_invoke() const { _mutex.lock(); }
				void unlock_invoke() const { _mutex.unlock(); }
			};
		};


		struct own_mutex
		{
			class lock_primitive
			{
			private:
				mutable std::mutex		_mutex;

			public:
				std::mutex& get_primitive() const { return _mutex; }

				void lock_connect() const { _mutex.lock(); }
				void unlock_connect() const { _mutex.unlock(); }

				void lock_invoke() const
				{
					if (_mutex.try_lock())
					{
						_mutex.unlock();
						throw std::runtime_error("A nonrecursive mutex should be locked outside of signal::operator()!");

					}
				}

				void unlock_invoke() const { }
			};
		};


		struct none
		{
			class lock_primitive
			{
			public:
				void get_primitive() const { }

				void lock_connect() const { }
				void unlock_connect() const { }

				void lock_invoke() const { }
				void unlock_invoke() const { }
			};
		};
	}


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	namespace state_populating
	{
		struct populator_only
		{
			template < typename Signature_ >
			class handler_processor
			{
				using handler_processor_func = std::function<void(const std::function<Signature_>&)>;

			private:
				handler_processor_func		_populator;

			public:
				handler_processor(handler_processor_func populator = &populator_only::handler_processor<Signature_>::empty_handler)
					: _populator(populator)
				{ }

				void populate_state(const std::function<Signature_>& handler) const { _populator(handler); }
				void withdraw_state(const std::function<Signature_>& handler) const { }

				static void empty_handler(const std::function<Signature_>&) { }
			};
		};


		struct populator_and_withdrawer
		{
			template < typename Signature_ >
			class handler_processor
			{
				using handler_processor_func = std::function<void(const std::function<Signature_>&)>;

			private:
				handler_processor_func		_populator;
				handler_processor_func		_withdrawer;

			public:
				handler_processor(handler_processor_func populator = &populator_only::handler_processor<Signature_>::empty_handler,
							handler_processor_func withdrawer = &populator_only::handler_processor<Signature_>::empty_handler)
					: _populator(populator), _withdrawer(withdrawer)
				{ }

				void populate_state(const std::function<Signature_>& handler) const { _populator(handler); }
				void withdraw_state(const std::function<Signature_>& handler) const { _withdrawer(handler); }

				static void empty_handler(const std::function<Signature_>&) { }
			};
		};


		struct none
		{
			template < typename Signature_ >
			struct handler_processor
			{
				void populate_state(const std::function<Signature_>& handler) const { }
				void withdraw_state(const std::function<Signature_>& handler) const { }
			};
		};
	}


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	namespace handlers_container
	{
		struct list
		{
			template < typename HandlerInfo_ >
			class handlers_container
			{
			public:
				using handlers_list = std::list<HandlerInfo_>;
				using handler_id = typename handlers_list::iterator;

			private:
				handlers_list		_handlers;

			public:
				const HandlerInfo_& get_handler_info(handler_id id)
				{ return *id; }

				handler_id add_handler(HandlerInfo_ handlerInfo)
				{
					_handlers.push_back(handlerInfo);
					handler_id it = _handlers.end();
					--it;
					return it;
				}

				void erase_handler(handler_id id)
				{ _handlers.erase(id); }

				const handlers_list& get_container() const
				{ return _handlers; }
			};
		};
	}


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	namespace handlers_stack_container
	{
		struct vector
		{
			template < typename HandlerInfo_ >
			using handlers_stack_container = std::vector<HandlerInfo_>;
		};
	}


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	namespace life_assurance
	{
		struct life_tokens
		{
			struct life_checker
			{
				life_token::checker		checker;
				life_checker(const life_token::checker& c) : checker(c) { }
			};

			struct execution_guard
			{
				life_token::execution_guard		guard;
				execution_guard(const life_checker& c) : guard(c.checker) { }
				bool is_alive() const { return guard.is_alive(); }
			};

			class life_assurance
			{
				union life_token_storage
				{
					life_token		token;

					life_token_storage(life_token_storage&& other) noexcept
					{
						new(&token) life_token(std::move(other.token));
						other.token.~life_token();
					}

					life_token_storage() { new(&token) life_token(); }
					~life_token_storage() { }
				};

				life_token_storage		_token_storage;

			public:
				life_checker get_life_checker() const { return life_checker(_token_storage.token); }
				void release() { _token_storage.token.~life_token(); }
			};
		};


		struct none
		{
			struct life_checker
			{ };

			struct execution_guard
			{
				execution_guard(const life_checker&) { }
				bool is_alive() const { return true; }
			};

			struct life_assurance
			{
				life_checker get_life_checker() const { return life_checker(); }
				void release() { }
			};
		};
	}


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	namespace impl_storage
	{
		struct inplace
		{
			template < typename ExceptionHandler_, typename LockPrimitive_, typename HandlerProcessor_, typename HandlersContainer_ >
			class storage : private ExceptionHandler_, private LockPrimitive_, private HandlerProcessor_, private HandlersContainer_
			{
				template< typename T_, typename... Args_ >
				using con = std::is_constructible<T_, Args_...>;

			public:
				storage() { }

#define DETAIL_WIGWAG_STORAGE_CTOR_ENABLER(...) typename std::enable_if<__VA_ARGS__, detail::enabler>::type e = detail::enabler()

				template < typename T_ > storage(T_ eh, DETAIL_WIGWAG_STORAGE_CTOR_ENABLER(con<ExceptionHandler_, T_&&>::value)) : ExceptionHandler_(std::move(eh)) { }
				template < typename T_ > storage(T_ lp, DETAIL_WIGWAG_STORAGE_CTOR_ENABLER(con<LockPrimitive_, T_&&>::value)) : LockPrimitive_(std::move(lp)) { }
				template < typename T_ > storage(T_ hp, DETAIL_WIGWAG_STORAGE_CTOR_ENABLER(con<HandlerProcessor_, T_&&>::value)) : HandlerProcessor_(std::move(hp)) { }
				template < typename T_ > storage(T_ hc, DETAIL_WIGWAG_STORAGE_CTOR_ENABLER(con<HandlersContainer_, T_&&>::value)) : HandlersContainer_(std::move(hc)) { }

				template < typename T_, typename U_ >
				storage(T_ eh, U_ lp, DETAIL_WIGWAG_STORAGE_CTOR_ENABLER(con<ExceptionHandler_, T_&&>::value && con<LockPrimitive_, U_&&>::value))
					: ExceptionHandler_(std::move(eh)), LockPrimitive_(std::move(lp))
				{ }

				template < typename T_, typename U_ >
				storage(T_ eh, U_ hp, DETAIL_WIGWAG_STORAGE_CTOR_ENABLER(con<ExceptionHandler_, T_&&>::value && con<HandlerProcessor_, U_&&>::value))
					: ExceptionHandler_(std::move(eh)), HandlerProcessor_(std::move(hp))
				{ }

				template < typename T_, typename U_ >
				storage(T_ eh, U_ hc, DETAIL_WIGWAG_STORAGE_CTOR_ENABLER(con<ExceptionHandler_, T_&&>::value && con<HandlersContainer_, U_&&>::value))
					: ExceptionHandler_(std::move(eh)), LockPrimitive_(std::move(hc))
				{ }

				template < typename T_, typename U_ >
				storage(T_ lp, U_ hp, DETAIL_WIGWAG_STORAGE_CTOR_ENABLER(con<LockPrimitive_, T_&&>::value && con<HandlerProcessor_, U_&&>::value))
					: LockPrimitive_(std::move(lp)), HandlerProcessor_(std::move(hp))
				{ }

				template < typename T_, typename U_ >
				storage(T_ lp, U_ hc, DETAIL_WIGWAG_STORAGE_CTOR_ENABLER(con<LockPrimitive_, T_&&>::value && con<HandlersContainer_, U_&&>::value))
					: LockPrimitive_(std::move(lp)), HandlersContainer_(std::move(hc))
				{ }

				template < typename T_, typename U_ >
				storage(T_ hp, U_ hc, DETAIL_WIGWAG_STORAGE_CTOR_ENABLER(con<HandlerProcessor_, T_&&>::value && con<HandlersContainer_, U_&&>::value))
					: HandlerProcessor_(std::move(hp)), HandlersContainer_(std::move(hc))
				{ }

				template < typename T_, typename U_, typename V_ >
				storage(T_ lp, U_ hp, V_ hc, DETAIL_WIGWAG_STORAGE_CTOR_ENABLER(con<LockPrimitive_, T_&&>::value && con<HandlerProcessor_, U_&&>::value && con<HandlersContainer_, V_&&>::value))
					: LockPrimitive_(std::move(lp)), HandlerProcessor_(std::move(hp)), HandlersContainer_(std::move(hc))
				{ }

				template < typename T_, typename U_, typename V_ >
				storage(T_ eh, U_ hp, V_ hc, DETAIL_WIGWAG_STORAGE_CTOR_ENABLER(con<ExceptionHandler_, T_&&>::value && con<HandlerProcessor_, U_&&>::value && con<HandlersContainer_, V_&&>::value))
					: ExceptionHandler_(std::move(eh)), HandlerProcessor_(std::move(hp)), HandlersContainer_(std::move(hc))
				{ }

				template < typename T_, typename U_, typename V_ >
				storage(T_ eh, U_ lp, V_ hc, DETAIL_WIGWAG_STORAGE_CTOR_ENABLER(con<ExceptionHandler_, T_&&>::value && con<LockPrimitive_, U_&&>::value && con<HandlersContainer_, V_&&>::value))
					: ExceptionHandler_(std::move(eh)), LockPrimitive_(std::move(lp)), HandlersContainer_(std::move(hc))
				{ }

				template < typename T_, typename U_, typename V_ >
				storage(T_ eh, U_ lp, V_ hp, DETAIL_WIGWAG_STORAGE_CTOR_ENABLER(con<ExceptionHandler_, T_&&>::value && con<LockPrimitive_, U_&&>::value && con<HandlerProcessor_, V_&&>::value))
					: ExceptionHandler_(std::move(eh)), LockPrimitive_(std::move(lp)), HandlerProcessor_(std::move(hp))
				{ }

#undef DETAIL_WIGWAG_STORAGE_CTOR_ENABLER

				storage(ExceptionHandler_ eh, LockPrimitive_ lp, HandlerProcessor_ hp, HandlersContainer_ hc) : ExceptionHandler_(std::move(eh)), LockPrimitive_(std::move(lp)), HandlerProcessor_(std::move(hp)), HandlersContainer_(std::move(hc)) { }

				HandlersContainer_& get_handlers_container() { return *this; }
				const HandlersContainer_& get_handlers_container() const { return *this; }

				const LockPrimitive_& get_lock_primitive() const { return *this; }
				const ExceptionHandler_& get_exception_handler() const { return *this; }
				const HandlerProcessor_& get_handler_processor() const { return *this; }
			};
		};


		struct shared
		{
			template < typename ExceptionHandler_, typename LockPrimitive_, typename HandlerProcessor_, typename HandlersContainer_ >
			class storage
			{
			private:
				using impl = inplace::storage<ExceptionHandler_, LockPrimitive_, HandlerProcessor_, HandlersContainer_>;

				std::shared_ptr<impl>	_impl;

			public:
				template < typename... Args_ >
				storage(Args_... args)
					: _impl(new impl(std::forward<Args_>(args)...))
				{ }

				HandlersContainer_& get_handlers_container() { return _impl->get_handlers_container(); }
				const HandlersContainer_& get_handlers_container() const { return _impl->get_handlers_container(); }

				const LockPrimitive_& get_lock_primitive() const { return _impl->get_lock_primitive(); }
				const ExceptionHandler_& get_exception_handler() const { return _impl->get_exception_handler(); }
				const HandlerProcessor_& get_handler_processor() const { return _impl->get_handler_processor(); }
			};

			template < typename ExceptionHandlingPolicy_, typename LockPrimitive_, typename HandlerProcessor_,typename HandlersContainer_ >
			using storage_ref = storage<ExceptionHandlingPolicy_, LockPrimitive_, HandlerProcessor_, HandlersContainer_>;
		};
	}

}

#endif
