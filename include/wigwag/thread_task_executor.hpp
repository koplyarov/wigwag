#ifndef WIGWAG_THREAD_TASK_EXECUTOR_HPP
#define WIGWAG_THREAD_TASK_EXECUTOR_HPP

// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <wigwag/detail/policies_concepts.hpp>
#include <wigwag/detail/policy_picker.hpp>
#include <wigwag/policies.hpp>
#include <wigwag/task_executor.hpp>

#include <queue>
#include <thread>


namespace wigwag
{

#include <wigwag/detail/disable_warnings.hpp>

    namespace detail
    {
        using thread_task_executor_policies_config = policies_config<
                policies_config_entry<exception_handling::policy_concept, wigwag::exception_handling::default_>
            >;
    }


    template < typename... Policies_ >
    class basic_thread_task_executor :
        public task_executor,
        private detail::policy_picker<detail::exception_handling::policy_concept, detail::thread_task_executor_policies_config, Policies_...>::type
    {
        using exception_handling_policy = typename detail::policy_picker<detail::exception_handling::policy_concept, detail::thread_task_executor_policies_config, Policies_...>::type;

        using task_queue = std::queue<std::function<void()>>;

    private:
        task_queue                  _tasks;
        bool                        _alive;
        std::mutex                  _mutex;
        std::condition_variable     _cv;
        std::thread                 _thread;

    public:
        template < typename... Args_ >
        basic_thread_task_executor(Args_&... args)
            : exception_handling_policy(std::forward<Args_>(args)...), _alive(true)
        { _thread = std::thread(&basic_thread_task_executor::thread_func, this); }

        ~basic_thread_task_executor()
        {
            {
                std::lock_guard<std::mutex> l(_mutex);
                _alive = false;
                _cv.notify_all();
            }
            if (_thread.joinable())
                _thread.join();
        }

        virtual void add_task(std::function<void()> task)
        {
            std::lock_guard<std::mutex> l(_mutex);
            bool need_wakeup = _tasks.empty();
            _tasks.push(std::move(task));
            if (need_wakeup)
                _cv.notify_all();
        }

    private:
        void thread_func()
        {
            std::unique_lock<std::mutex> l(_mutex);
            while (_alive || !_tasks.empty())
            {
                if (_tasks.empty())
                {
                    _cv.wait(l);
                    continue;
                }

                exception_handling_policy::handle_exceptions([&]() {
                        std::function<void()> task;
                        std::swap(_tasks.front(), task);
                        _tasks.pop();

                        l.unlock();
                        auto sg = detail::at_scope_exit([&] { l.lock(); } );

                        task();
                    } );
            }
        }
    };


    using thread_task_executor = basic_thread_task_executor<>;


#include <wigwag/detail/enable_warnings.hpp>

}

#endif
