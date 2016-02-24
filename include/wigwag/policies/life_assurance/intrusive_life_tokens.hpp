#ifndef WIGWAG_POLICIES_LIFE_ASSURANCE_INTRUSIVE_LIFE_TOKENS_HPP
#define WIGWAG_POLICIES_LIFE_ASSURANCE_INTRUSIVE_LIFE_TOKENS_HPP

// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <wigwag/detail/annotations.hpp>
#include <wigwag/detail/config.hpp>

#include <atomic>
#include <condition_variable>
#include <limits>
#include <mutex>


namespace wigwag {
namespace life_assurance
{

#include <wigwag/detail/disable_warnings.hpp>

	struct intrusive_life_tokens
	{
		class life_assurance;
		class life_checker;
		class execution_guard;


		class shared_data
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

			void reset_life_assurance(const shared_data& sd)
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

			const shared_data*								_sd;
			detail::intrusive_ptr<const life_assurance>		_la;

		public:
			life_checker(const shared_data& sd, const life_assurance& la) noexcept
				: _sd(&sd), _la(&la)
			{ la.add_ref(); }
		};

		class execution_guard
		{
			const shared_data&								_sd;
			detail::intrusive_ptr<const life_assurance>		_la;
			life_assurance::int_type						_alive;

		public:
			execution_guard(const life_checker& c)
				: _sd(*c._sd), _la(c._la), _alive(++c._la->_lock_counter_and_alive_flag & life_assurance::alive_flag)
			{
				if (!_alive)
					unlock();
			}

			execution_guard(const shared_data& sd, const life_assurance& la)
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

			life_assurance::int_type is_alive() const noexcept
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

#include <wigwag/detail/enable_warnings.hpp>

}}

#endif
