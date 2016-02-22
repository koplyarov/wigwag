#ifndef WIGWAG_POLICIES_THREADING_SHARED_RECURSIVE_MUTEX_HPP
#define WIGWAG_POLICIES_THREADING_SHARED_RECURSIVE_MUTEX_HPP

// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <memory>
#include <thread>


namespace wigwag {
namespace threading
{

#include <wigwag/detail/disable_warnings.hpp>

	struct shared_recursive_mutex
	{
		class lock_primitive
		{
		private:
			std::shared_ptr<std::recursive_mutex>	_mutex;

		public:
			lock_primitive(const std::shared_ptr<std::recursive_mutex>& mutex)
				: _mutex(mutex)
			{ }

			std::shared_ptr<std::recursive_mutex> get_primitive() const noexcept { return _mutex; }

			void lock_nonrecursive() const { _mutex->lock(); }
			void unlock_nonrecursive() const { _mutex->unlock(); }

			void lock_recursive() const { _mutex->lock(); }
			void unlock_recursive() const { _mutex->unlock(); }
		};
	};

#include <wigwag/detail/enable_warnings.hpp>

}}

#endif
