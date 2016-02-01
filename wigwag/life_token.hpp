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


#include <atomic>
#include <condition_variable>
#include <limits>
#include <memory>


namespace wigwag
{

	class life_token
	{
	private:
		using int_type = unsigned int;
		static const int_type alive_flag = ((int_type)1) << (std::numeric_limits<int_type>::digits - 1);

		struct impl
		{
			std::atomic<int_type>		lock_counter_and_alive_flag;
			bool						unblock;
			std::condition_variable		cond_var;
			std::mutex					mutex;

			impl() : lock_counter_and_alive_flag(alive_flag), unblock(false) { }
		};
		using impl_ptr = std::shared_ptr<impl>;

	public:
		class checker;
		class execution_guard;

		impl_ptr		_impl;

	public:
		life_token()
			: _impl(std::make_shared<impl>())
		{ }

		~life_token()
		{
			_impl->lock_counter_and_alive_flag -= alive_flag;
			std::unique_lock<std::mutex> l(_impl->mutex);
			while (!_impl->unblock)
				_impl->cond_var.wait(l);
		}
	};


	class life_token::checker
	{
		friend class execution_guard;

	private:
		impl_ptr		_impl;

	public:
		checker(const life_token& token)
			: _impl(token._impl)
		{ }
	};


	class life_token::execution_guard
	{
	private:
		impl_ptr		_impl;
		bool			_alive;

	public:
		execution_guard(const life_token::checker& checker)
			: _impl(checker._impl)
		{
			int_type i = ++_impl->lock_counter_and_alive_flag;
			_alive = i & alive_flag;

			if (!_alive)
				dec_lock_counter();
		}

		~execution_guard()
		{
			if (_alive)
				dec_lock_counter();
		}

		bool is_alive() const
		{ return _alive; }

	private:
		void dec_lock_counter()
		{
			int_type i = --_impl->lock_counter_and_alive_flag;
			if (i == 0)
			{
				std::unique_lock<std::mutex> l(_impl->mutex);
				if (!_impl->unblock)
				{
					_impl->unblock = true;
					_impl->cond_var.notify_all();
				}
			}
		}
	};

}

#endif
