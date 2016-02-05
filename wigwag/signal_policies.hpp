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


#include <wigwag/life_token.hpp>

#include <list>
#include <memory>
#include <mutex>
#include <vector>


namespace wigwag
{

	namespace exception_handling
	{
		struct rethrow
		{
			void handle_std_exception(const std::exception& ex) const { throw; }
			void handle_unknown_exception() const { throw; }
		};
	}


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

			struct life_assurance
			{
				life_token		token;
				life_checker get_life_checker() const { return life_checker(token); }
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
			};
		};
	}


	namespace handlers_storage
	{
		struct shared_list
		{
			template < typename Signature_, typename ThreadingPolicy_, typename ExceptionHandlingPolicy_, typename LifeAssurancePolicy_ >
			class storage
			{
			public:
				using life_checker = typename LifeAssurancePolicy_::life_checker;
				using lock_primitive = typename ThreadingPolicy_::lock_primitive;
				using handler_type = std::function<Signature_>;

				struct handler_info : public life_checker
				{
					handler_type		handler;

					handler_info(life_checker lc, handler_type h)
						: life_checker(lc), handler(h)
					{ }

					const life_checker& get_life_checker() const { return *this; }
					const handler_type& get_handler() const { return handler; }
				};

				using handlers_stack_container = std::vector<handler_info>;

				using handlers_list = std::list<handler_info>;
				using handler_id = typename handlers_list::iterator;

			private:
				struct impl : public lock_primitive, public ExceptionHandlingPolicy_
				{
					handlers_list		handlers;
				};

				std::shared_ptr<impl>	_impl;

			public:
				storage() : _impl(new impl) { }

				handler_id add_handler(handler_type handler, life_checker lc)
				{
					_impl->handlers.push_back(handler_info{lc, handler});
					handler_id it = _impl->handlers.end();
					--it;
					return it;
				}

				void erase_handler(handler_id id)
				{ _impl->handlers.erase(id); }

				const handlers_list& get_handlers() const { return _impl->handlers; }

				const lock_primitive& get_lock_primitive() const { return *_impl; }
				const ExceptionHandlingPolicy_& get_exception_handler() const { return *_impl; }
			};

			template < typename Signature_, typename ThreadingPolicy_, typename ExceptionHandlingPolicy_, typename LifeAssurancePolicy_ >
			using storage_ref = storage<Signature_, ThreadingPolicy_, ExceptionHandlingPolicy_, LifeAssurancePolicy_>;
		};
	}


	namespace threading
	{
		template < typename MutexType_ >
		struct own_recursive_mutex
		{
			class lock_primitive
			{
			private:
				mutable MutexType_		_mutex;

			public:
				void lock_connect() const { _mutex.lock(); }
				void unlock_connect() const { _mutex.unlock(); }

				void lock_invoke() const { _mutex.lock(); }
				void unlock_invoke() const { _mutex.unlock(); }
			};
		};


		template < typename MutexType_ >
		struct own_mutex
		{
			class lock_primitive
			{
			private:
				mutable MutexType_		_mutex;

			public:
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


		struct threadless
		{
			class lock_primitive
			{
			public:
				void lock_connect() const { }
				void unlock_connect() const { }

				void lock_invoke() const { }
				void unlock_invoke() const { }
			};
		};
	}

}

#endif
