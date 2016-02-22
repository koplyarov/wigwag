#ifndef UTILS_UTILS_HPP
#define UTILS_UTILS_HPP

// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <chrono>
#include <thread>


namespace wigwag
{

	class thread
	{
	public:
		using thread_func = std::function<void(const std::atomic<bool>&)>;

	private:
		std::atomic<bool>		_alive;
		thread_func				_thread_func;
		std::string				_error_message;
		std::thread				_impl;

	public:
		thread(const thread_func& f)
			:	_alive(true),
				_thread_func(f)
		{ _impl = std::thread(std::bind(&thread::func, this)); }

		~thread()
		{
			_alive = false;
			if (_impl.joinable())
				_impl.join();
			if (!_error_message.empty())
				TS_FAIL(("Uncaught exception in thread: " + _error_message).c_str());
		}

		static void sleep(int64_t ms)
		{ std::this_thread::sleep_for(std::chrono::milliseconds(ms)); }

	private:
		void func()
		{
			try
			{ _thread_func(_alive); }
			catch (const std::exception& ex)
			{ _error_message = ex.what(); }
		}
	};


	template < typename Lockable_ >
	std::unique_lock<Lockable_> lock(Lockable_& lockable)
	{ return std::unique_lock<Lockable_>(lockable); }

}

#endif
