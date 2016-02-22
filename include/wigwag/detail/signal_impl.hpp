#ifndef WIGWAG_DETAIL_SIGNAL_IMPL_HPP
#define WIGWAG_DETAIL_SIGNAL_IMPL_HPP

// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <wigwag/detail/async_handler.hpp>
#include <wigwag/detail/listenable_impl.hpp>
#include <wigwag/detail/signal_connector_impl.hpp>


namespace wigwag {
namespace detail
{

#include <wigwag/detail/disable_warnings.hpp>

	template <
			typename Signature_,
			typename ExceptionHandlingPolicy_,
			typename ThreadingPolicy_,
			typename StatePopulatingPolicy_,
			typename LifeAssurancePolicy_
		>
	class signal_impl
		:	public signal_connector_impl<Signature_>,
			private listenable_impl<std::function<Signature_>, ExceptionHandlingPolicy_, ThreadingPolicy_, StatePopulatingPolicy_, LifeAssurancePolicy_>
	{
		using listenable_base = listenable_impl<std::function<Signature_>, ExceptionHandlingPolicy_, ThreadingPolicy_, StatePopulatingPolicy_, LifeAssurancePolicy_>;

		using handler_type = std::function<Signature_>;

		using handler_node = typename listenable_base::handler_node;
		using lock_primitive = typename listenable_base::lock_primitive;
		using life_checker = typename listenable_base::life_checker;
		using execution_guard = typename listenable_base::execution_guard;

	public:
		template < typename... Args_ >
		signal_impl(Args_&&... args)
			: listenable_base(std::forward<Args_>(args)...)
		{ }

		void finalize_nodes()
		{ listenable_base::finalize_nodes(); }

		const lock_primitive& get_lock_primitive() const
		{ return listenable_base::get_lock_primitive(); }

		virtual void add_ref() { listenable_base::add_ref(); }
		virtual void release() { listenable_base::release(); }

		virtual token connect(const handler_type& handler)
		{ return listenable_base::connect(handler); }

		virtual token connect(const std::shared_ptr<task_executor>& worker, const handler_type& handler)
		{
			this->get_lock_primitive().lock_nonrecursive();
			auto sg = detail::at_scope_exit([&] { this->get_lock_primitive().unlock_nonrecursive(); } );

			add_ref();
			intrusive_ptr<listenable_base> self(this);

			return token::create<handler_node>(self,
					[&](const life_checker& lc) {
						async_handler<Signature_, LifeAssurancePolicy_> real_handler(worker, lc, handler);
						this->get_exception_handler().handle_exceptions([&] { this->get_handler_processor().populate_state(real_handler); });
						return real_handler;
					});
		}

		template < typename... Args_ >
		void invoke(Args_&&... args)
		{
			this->get_lock_primitive().lock_recursive();
			auto sg = detail::at_scope_exit([&] { this->get_lock_primitive().unlock_recursive(); } );
			for (auto it = this->_handlers.begin(); it != this->_handlers.end();)
			{
				if (it->should_be_finalized())
				{
					(it++)->finalize_node();
					continue;
				}

				execution_guard g(listenable_base::get_life_assurance_shared_data(), it->get_life_assurance());
				if (g.is_alive())
					this->get_exception_handler().handle_exceptions(it->get_handler(), std::forward<Args_>(args)...);
				++it;
			}
		}
	};

#include <wigwag/detail/enable_warnings.hpp>

}}

#endif
