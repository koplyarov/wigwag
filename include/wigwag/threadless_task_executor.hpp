#ifndef WIGWAG_THREADLESS_TASK_EXECUTOR_HPP
#define WIGWAG_THREADLESS_TASK_EXECUTOR_HPP

// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <wigwag/policies.hpp>
#include <wigwag/task_executor.hpp>

#include <queue>
#include <thread>


namespace wigwag
{

#include <wigwag/detail/disable_warnings.hpp>

	template < typename ExceptionHandlingPolicy_ = exception_handling::default_, typename ThreadingPolicy_ = threading::default_ >
	class basic_threadless_task_executor : public task_executor, private ExceptionHandlingPolicy_
	{
		using task_queue = std::queue<std::function<void()>>;

		using lock_primitive = typename ThreadingPolicy_::lock_primitive;

	private:
		task_queue				_tasks;
		lock_primitive			_lp;

	public:
		template < typename... Args_ >
		basic_threadless_task_executor(Args_&... args)
			: ExceptionHandlingPolicy_(std::forward<Args_>(args)...)
		{ }

		~basic_threadless_task_executor()
		{ }

		virtual void add_task(const std::function<void()>& task)
		{
			_lp.lock_nonrecursive();
			auto sg = detail::at_scope_exit([&] { _lp.unlock_nonrecursive(); } );

			_tasks.push(task);
		}

		void process_tasks()
		{
			_lp.lock_nonrecursive();
			auto sg = detail::at_scope_exit([&] { _lp.unlock_nonrecursive(); } );

			while (!_tasks.empty())
			{
				ExceptionHandlingPolicy_::handle_exceptions([&]() {
						std::function<void()> task = _tasks.front();
						_tasks.pop();

						_lp.unlock_nonrecursive();
						auto sg = detail::at_scope_exit([&] { _lp.lock_nonrecursive(); } );

						task();
					} );
			}
		}
	};


	using threadless_task_executor = basic_threadless_task_executor<>;


#include <wigwag/detail/enable_warnings.hpp>

}

#endif
