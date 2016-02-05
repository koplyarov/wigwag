#ifndef WIGWAG_SIGNAL_HPP
#define WIGWAG_SIGNAL_HPP

// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <wigwag/detail/at_scope_exit.hpp>
#include <wigwag/life_token.hpp>
#include <wigwag/signal_policies.hpp>
#include <wigwag/token.hpp>

#include <list>


namespace wigwag
{

	template < typename Signature_, typename ThreadingPolicy_, typename ExceptionHandlingPolicy_, typename HandlersStoragePolicy_, typename LifeAssurancePolicy_ >
	class basic_signal
	{
	public:
		using handler_type = std::function<Signature_>;
		using populator_type = std::function<void(const std::function<Signature_>&)>;

		using lock_primitive = typename ThreadingPolicy_::lock_primitive;

		using life_assurance = typename LifeAssurancePolicy_::life_assurance;
		using life_checker = typename LifeAssurancePolicy_::life_checker;
		using execution_guard = typename LifeAssurancePolicy_::execution_guard;

		using storage = typename HandlersStoragePolicy_::template storage<Signature_, ThreadingPolicy_, ExceptionHandlingPolicy_, LifeAssurancePolicy_>;
		using storage_ref = typename HandlersStoragePolicy_::template storage_ref<Signature_, ThreadingPolicy_, ExceptionHandlingPolicy_, LifeAssurancePolicy_>;
		using handlers_stack_container = typename storage::handlers_stack_container;
		using handler_id = typename storage::handler_id;

	private:

		struct connection : public token::implementation, public life_assurance
		{
			storage_ref		_storage_ref;
			handler_id		_id;

		public:
			connection(life_assurance&& la, storage_ref sr, handler_id id)
				: life_assurance(std::move(la)), _storage_ref(sr), _id(id)
			{ }

			~connection()
			{
				_storage_ref.get_lock_primitive().lock_connect();
				auto sg = at_scope_exit([&]() { _storage_ref.get_lock_primitive().unlock_connect(); } );
				_storage_ref.erase_handler(_id);
			}
		};

	private:
		storage		_storage;

	public:
		basic_signal() { }
		basic_signal(const basic_signal&) = delete;
		basic_signal& operator = (const basic_signal&) = delete;

		token connect(const handler_type& handler)
		{
			_storage.get_lock_primitive().lock_connect();
			auto sg = at_scope_exit([&]() { _storage.get_lock_primitive().unlock_connect(); } );

			life_assurance la;
			auto id = _storage.add_handler(handler, la.get_life_checker());
			return token::create<connection>(std::move(la), storage_ref(_storage), id);
		}

		template < typename... Args >
		void operator() (Args&&... args) const
		{
			handlers_stack_container handlers_copy;

			{
				_storage.get_lock_primitive().lock_invoke();
				auto sg = at_scope_exit([&]() { _storage.get_lock_primitive().unlock_invoke(); } );

				handlers_copy.assign(_storage.get_handlers().begin(), _storage.get_handlers().end());
			}

			for (const auto& h : handlers_copy)
			{
				execution_guard g(h.get_life_checker());
				if (g.is_alive())
				{
					try
					{ h.get_handler()(std::forward<Args>(args)...); }
					catch (const std::exception& ex)
					{ _storage.get_exception_handler().handle_std_exception(ex); }
					catch (...)
					{ _storage.get_exception_handler().handle_unknown_exception(); }
				}
			}
		}
	};


	template < typename Signature_ >
	using signal = basic_signal <
						Signature_,
						threading::own_recursive_mutex<std::mutex>,
						exception_handling::rethrow,
						handlers_storage::shared_list,
						life_assurance::life_tokens
					>;

}

#endif
