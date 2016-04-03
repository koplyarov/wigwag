#ifndef WIGWAG_DETAIL_ASYNC_HANDLER_HPP
#define WIGWAG_DETAIL_ASYNC_HANDLER_HPP

// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <wigwag/task_executor.hpp>

#include <memory>


namespace wigwag {
namespace detail
{

#include <wigwag/detail/disable_warnings.hpp>

	template < typename Signature_, typename LifeAssurancePolicy_ >
	class async_handler
	{
		using life_checker = typename LifeAssurancePolicy_::life_checker;
		using execution_guard = typename LifeAssurancePolicy_::execution_guard;

	private:
		std::shared_ptr<task_executor>	_worker;
		life_checker					_life_checker;
		std::function<Signature_>		_func;

	public:
		async_handler(const std::shared_ptr<task_executor>& worker, const life_checker& checker, const std::function<Signature_>& func)
			: _worker(worker), _life_checker(checker), _func(func)
		{ }

		template < typename... Args_ >
		void operator() (Args_&&... args) const
		{ _worker->add_task(std::bind(&async_handler::invoke_func<Args_&...>, _life_checker, _func, std::forward<Args_>(args)...)); }

	private:
		template < typename... Args_ >
		static void invoke_func(life_checker checker, const std::function<Signature_>& func, Args_&&... args)
		{
			execution_guard g(checker);
			if (g.is_alive())
				func(std::forward<Args_>(args)...);
		}
	};

#include <wigwag/detail/enable_warnings.hpp>

}}

#endif
