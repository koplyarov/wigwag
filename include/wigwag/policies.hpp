#ifndef WIGWAG_POLICIES_HPP
#define WIGWAG_POLICIES_HPP

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

#include <iostream>
#include <list>
#include <memory>
#include <mutex>
#include <utility>
#include <vector>

#include <stdio.h>


namespace wigwag
{

#include <wigwag/detail/disable_warnings.hpp>

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
				std::recursive_mutex& get_primitive() const noexcept { return _mutex; }

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
				std::mutex& get_primitive() const noexcept { return _mutex; }

				void lock_connect() const { _mutex.lock(); }
				void unlock_connect() const { _mutex.unlock(); }

				void lock_invoke() const noexcept { }
				void unlock_invoke() const noexcept { }
			};
		};


		struct none
		{
			class lock_primitive
			{
			public:
				void get_primitive() const noexcept { }

				void lock_connect() const noexcept { }
				void unlock_connect() const noexcept { }

				void lock_invoke() const noexcept { }
				void unlock_invoke() const noexcept { }
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
			template < typename HandlerType_ >
			class handler_processor
			{
				using handler_processor_func = std::function<void(const HandlerType_&)>;

			private:
				handler_processor_func		_populator;

			public:
				handler_processor(handler_processor_func populator = &populator_only::handler_processor<HandlerType_>::empty_handler)
					: _populator(populator)
				{ }

				void populate_state(const HandlerType_& handler) const { _populator(handler); }

				template < typename LockPrimitive_ >
				void withdraw_state(LockPrimitive_&, const HandlerType_&) const noexcept { }

				static void empty_handler(const HandlerType_&) noexcept { }
			};
		};


		struct populator_and_withdrawer
		{
			template < typename HandlerType_ >
			class handler_processor
			{
				using handler_processor_func = std::function<void(const HandlerType_&)>;

			private:
				handler_processor_func		_populator;
				handler_processor_func		_withdrawer;

			public:
				handler_processor(handler_processor_func populator = &populator_only::handler_processor<HandlerType_>::empty_handler,
							handler_processor_func withdrawer = &populator_only::handler_processor<HandlerType_>::empty_handler)
					: _populator(populator), _withdrawer(withdrawer)
				{ }

				template < typename K_, typename V_ >
				handler_processor(const std::pair<K_, V_>& populator_and_withdrawer_pair, typename std::enable_if<std::is_constructible<handler_processor_func, K_>::value && std::is_constructible<handler_processor_func, V_>::value, wigwag::detail::enabler>::type e = wigwag::detail::enabler())
					: _populator(populator_and_withdrawer_pair.first), _withdrawer(populator_and_withdrawer_pair.second)
				{ }

				void populate_state(const HandlerType_& handler) const { _populator(handler); }

				template < typename LockPrimitive_ >
				void withdraw_state(LockPrimitive_& lp, const HandlerType_& handler) const
				{
					std::lock_guard<LockPrimitive_> l(lp);
					_withdrawer(handler);
				}

				static void empty_handler(const HandlerType_&) noexcept { }
			};
		};


		struct none
		{
			template < typename HandlerType_ >
			struct handler_processor
			{
				void populate_state(const HandlerType_&) const noexcept { }

				template < typename LockPrimitive_ >
				void withdraw_state(LockPrimitive_&, const HandlerType_&) const noexcept { }
			};
		};
	}


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	namespace life_assurance
	{
		struct intrusive_life_tokens;
		using default_ = intrusive_life_tokens;


		struct intrusive_life_tokens
		{
			class life_assurance;
			class life_checker;
			class execution_guard;

			class signal_data
			{
				friend class life_assurance;
				friend class execution_guard;

			private:
				mutable std::condition_variable		_cond_var;
				mutable std::mutex					_mutex;
			};

			class life_assurance
			{
				friend class life_checker;
				friend class execution_guard;

				using int_type = unsigned int;
				static const int_type alive_flag = ((int_type)1) << (std::numeric_limits<int_type>::digits - 1);

				mutable std::atomic<int_type>		_lock_counter_and_alive_flag;
				mutable std::atomic<int>			_ref_count;

			public:
				life_assurance()
					: _lock_counter_and_alive_flag(alive_flag), _ref_count(2) // One ref in signal, another in token
				{ }

				life_assurance(const life_assurance&) = delete;
				life_assurance& operator = (const life_assurance&) = delete;


				void add_ref() const
				{ ++_ref_count; }

				void release() const
				{
					if (--_ref_count == 0)
						WIGWAG_ASSERT(false, "Inconsistent reference counter!");

					WIGWAG_ANNOTATE_HAPPENS_BEFORE(this);
				}

				void reset_life_assurance(const signal_data& sd)
				{
					_lock_counter_and_alive_flag -= alive_flag;
					std::unique_lock<std::mutex> l(sd._mutex);
					while (_lock_counter_and_alive_flag != 0)
						sd._cond_var.wait(l);
				}

				bool node_deleted_on_finalize() const
				{ return true; }

				bool should_be_finalized() const
				{ return _ref_count == 1; }

				template < typename HandlerNode_ >
				void release_external_ownership(const HandlerNode_*)
				{ release(); }

				template < typename HandlerNode_ >
				void finalize(const HandlerNode_* node)
				{
					if (--_ref_count != 0)
						WIGWAG_ASSERT(false, "Inconsistent reference counter!");

					WIGWAG_ANNOTATE_HAPPENS_AFTER(this);
					WIGWAG_ANNOTATE_RELEASE(this);

					delete node;
				}
			};

			class life_checker
			{
				friend class execution_guard;

				const signal_data*								_sd;
				detail::intrusive_ptr<const life_assurance>		_la;

			public:
				life_checker(const signal_data& sd, const life_assurance& la) noexcept
					: _sd(&sd), _la(&la)
				{ la.add_ref(); }
			};

			class execution_guard
			{
				const signal_data&								_sd;
				detail::intrusive_ptr<const life_assurance>		_la;
				int												_alive;

			public:
				execution_guard(const life_checker& c)
					: _sd(*c._sd), _la(c._la), _alive(++c._la->_lock_counter_and_alive_flag & life_assurance::alive_flag)
				{
					if (!_alive)
						unlock();
				}

				execution_guard(const signal_data& sd, const life_assurance& la)
					: _sd(sd), _la(&la), _alive(++la._lock_counter_and_alive_flag & life_assurance::alive_flag)
				{
					la.add_ref();
					if (!_alive)
						unlock();
				}

				~execution_guard()
				{
					if (_alive)
						unlock();
				}

				int is_alive() const noexcept
				{ return _alive; }

			private:
				void unlock()
				{
					life_assurance::int_type i = --_la->_lock_counter_and_alive_flag;
					if (i == 0)
					{
						WIGWAG_ANNOTATE_HAPPENS_AFTER(&_la->_lock_counter_and_alive_flag);
						WIGWAG_ANNOTATE_RELEASE(&_la->_lock_counter_and_alive_flag);

						std::unique_lock<std::mutex> l(_sd._mutex);
						_sd._cond_var.notify_all();
					}
					else
						WIGWAG_ANNOTATE_HAPPENS_BEFORE(&_la->_lock_counter_and_alive_flag);
				}
			};
		};


		struct life_tokens
		{
			class life_checker;
			class execution_guard;

			class signal_data
			{ };

			class life_assurance
			{
				friend class life_checker;
				friend class execution_guard;

				life_token	_token;

			public:
				void reset_life_assurance(const signal_data&)
				{ _token.reset(); }

				bool node_deleted_on_finalize() const
				{ return false; }

				bool should_be_finalized() const
				{ return false; }

				template < typename HandlerNode_ >
				void release_external_ownership(const HandlerNode_* node)
				{ delete node; }

				template < typename HandlerNode_ >
				void finalize(const HandlerNode_*)
				{ }
			};

			class life_checker
			{
				friend class execution_guard;

				life_token::checker		_checker;

			public:
				life_checker(const signal_data&, const life_assurance& la) noexcept : _checker(la._token) { }
			};

			class execution_guard
			{
				life_token::execution_guard		_guard;

			public:
				execution_guard(const life_checker& c) : _guard(c._checker) { } // TODO: looks like noexcept here makes the code faster, check it on other machines
				execution_guard(const signal_data&, const life_assurance& la) : _guard(la._token) { }
				int is_alive() const noexcept { return _guard.is_alive(); }
			};
		};


		struct none
		{
			class signal_data
			{ };

			struct life_assurance
			{
				void reset_life_assurance(const signal_data&)
				{ }

				bool node_deleted_on_finalize() const
				{ return false; }

				bool should_be_finalized() const
				{ return false; }

				template < typename HandlerNode_ >
				void release_external_ownership(const HandlerNode_* node)
				{ delete node; }

				template < typename HandlerNode_ >
				void finalize(const HandlerNode_*)
				{ }
			};

			struct life_checker
			{
				life_checker(const signal_data&, const life_assurance&) noexcept { }
			};

			struct execution_guard
			{
				execution_guard(const life_checker&) noexcept { }
				execution_guard(const signal_data&, const life_assurance&) noexcept { }
				int is_alive() const noexcept { return true; }
			};
		};
	}

#include <wigwag/detail/enable_warnings.hpp>

}

#endif
