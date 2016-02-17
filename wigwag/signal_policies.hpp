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


#include <wigwag/detail/config.hpp>
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
		struct none;
		using default_ = none;


		struct none
		{
			template < typename Func_, typename... Args_ >
			void handle_exceptions(Func_&& func, Args_&&... args) const
			{ func(std::forward<Args_>(args)...); }
		};


#if !WIGWAG_NOEXCEPTIONS
		struct print_to_stderr
		{
			template < typename Func_, typename... Args_ >
			void handle_exceptions(Func_&& func, Args_&&... args) const
			{
				try
				{ func(std::forward<Args_>(args)...); }
				catch (const std::exception& ex)
				{ fprintf(stderr, "std::exception in signal handler: %s\n", ex.what()); }
				catch (...)
				{ fprintf(stderr, "Unknown exception in signal handler!\n"); }
			}
		};
#endif
	}


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	namespace threading
	{
		struct own_recursive_mutex;
		using default_ = own_recursive_mutex;


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
						WIGWAG_THROW("A nonrecursive mutex should be locked outside of signal::operator()!");
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
		struct populator_only;
		using default_ = populator_only;


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


	namespace life_assurance
	{
		struct life_tokens;
		using default_ = life_tokens;


		struct life_tokens
		{
			class life_checker;
			class execution_guard;

			class life_assurance
			{
				friend class life_checker;
				friend class execution_guard;

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
				void release() { _token_storage.token.~life_token(); }
			};

			class life_checker
			{
				friend class execution_guard;

				life_token::checker		checker;

			public:
				life_checker(const life_assurance& la) : checker(la._token_storage.token) { }
			};

			class execution_guard
			{
				life_token::execution_guard		guard;

			public:
				execution_guard(const life_checker& c) : guard(c.checker) { }
				execution_guard(const life_assurance& la) : guard(la._token_storage.token) { }
				bool is_alive() const { return guard.is_alive(); }
			};
		};


		struct none
		{
			struct life_assurance
			{
				void release() { }
			};

			struct life_checker
			{
				life_checker(const life_assurance&) {}
			};

			struct execution_guard
			{
				execution_guard(const life_checker&) { }
				execution_guard(const life_assurance&) { }
				bool is_alive() const { return true; }
			};
		};
	}


}

#endif
