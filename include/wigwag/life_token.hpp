#ifndef WIGWAG_LIFE_TOKEN_HPP
#define WIGWAG_LIFE_TOKEN_HPP

// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <wigwag/detail/annotations.hpp>

#include <atomic>
#include <condition_variable>
#include <limits>
#include <memory>


namespace wigwag
{

#include <wigwag/detail/disable_warnings.hpp>

	class life_token
	{
	private:
		using int_type = unsigned int;
		static const int_type alive_flag = ((int_type)1) << (std::numeric_limits<int_type>::digits - 1);

		struct impl
		{
			std::atomic<int_type>		lock_counter_and_alive_flag;
			std::condition_variable		cond_var;
			std::mutex					mutex;

			impl() : lock_counter_and_alive_flag(alive_flag) { }
		};
		using impl_ptr = std::shared_ptr<impl>;

	public:
		class checker;
		class execution_guard;

	private:
		impl_ptr		_impl;
		bool			_reset;

	public:
		life_token()
			: _impl(std::make_shared<impl>()), _reset(false)
		{ }

		life_token(life_token&& other) noexcept
			: _impl(other._impl)
		{ other._impl.reset(); }

		~life_token()
		{ reset(); }

		void reset()
		{
			if (_reset)
				return;

			_impl->lock_counter_and_alive_flag -= alive_flag;
			std::unique_lock<std::mutex> l(_impl->mutex);
			while (_impl->lock_counter_and_alive_flag != 0)
				_impl->cond_var.wait(l);

			_reset = true;
		}

		life_token(const life_token&) = delete;
		life_token& operator = (const life_token&) = delete;
	};


	class life_token::checker
	{
		friend class execution_guard;

	private:
		impl_ptr		_impl;

	public:
		checker(const life_token& token) noexcept
			: _impl(token._impl)
		{ }
	};


	class life_token::execution_guard
	{
	private:
		impl_ptr		_impl;
		int				_alive;

	public:
		execution_guard(const life_token& token)
			: _impl(token._impl), _alive(++_impl->lock_counter_and_alive_flag & alive_flag)
		{
			if (!_alive)
				unlock();
		}

		execution_guard(const life_token::checker& checker)
			: _impl(checker._impl), _alive(++_impl->lock_counter_and_alive_flag & alive_flag)
		{
			if (!_alive)
				unlock();
		}

		~execution_guard()
		{
			if (_alive)
				unlock();
		}

		int is_alive() const
		{ return _alive; }

	private:
		void unlock()
		{
			int_type i = --_impl->lock_counter_and_alive_flag;
			if (i == 0)
			{
				WIGWAG_ANNOTATE_HAPPENS_AFTER(&_impl->lock_counter_and_alive_flag);
				WIGWAG_ANNOTATE_RELEASE(&_impl->lock_counter_and_alive_flag);

				std::unique_lock<std::mutex> l(_impl->mutex);
				_impl->cond_var.notify_all();
			}
			else
				WIGWAG_ANNOTATE_HAPPENS_BEFORE(&_impl->lock_counter_and_alive_flag);
		}
	};

#include <wigwag/detail/enable_warnings.hpp>

}

#endif
