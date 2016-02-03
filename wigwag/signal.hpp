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


#include <wigwag/life_token.hpp>
#include <wigwag/signal_policies.hpp>
#include <wigwag/token.hpp>

#include <list>


namespace wigwag
{

	template < typename Signature_, typename ThreadingPolicy_, typename HandlersStoragePolicy_, typename LifeAssurancePolicy_ >
	class basic_signal
	{
	public:
		using handler_type = std::function<Signature_>;
		using populator_type = std::function<void(const std::function<Signature_>&)>;

		using lock_primitive = typename ThreadingPolicy_::lock_primitive;

		using life_assurance = typename LifeAssurancePolicy_::life_assurance;
		using life_checker = typename LifeAssurancePolicy_::life_checker;
		using execution_guard = typename LifeAssurancePolicy_::execution_guard;

		using handlers_stack_container = typename HandlersStoragePolicy_::template storage<Signature_, ThreadingPolicy_, LifeAssurancePolicy_>::handlers_stack_container;
		using handler_id = typename HandlersStoragePolicy_::template storage<Signature_, ThreadingPolicy_, LifeAssurancePolicy_>::handler_id;
		using storage = typename HandlersStoragePolicy_::template storage<Signature_, ThreadingPolicy_, LifeAssurancePolicy_>;
		using storage_ref = typename HandlersStoragePolicy_::template storage_ref<Signature_, ThreadingPolicy_, LifeAssurancePolicy_>;

	private:
		class lock_guard
		{
			const lock_primitive& _lp;

		public:
			lock_guard(const lock_primitive& lp) : _lp(lp) { _lp.lock(); }
			~lock_guard() { _lp.unlock(); }
		};

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
				lock_guard lg(_storage_ref.get_lock_primitive());
				_storage_ref.erase_handler(_id);
			}
		};

	private:
		storage		_storage;

	public:
		token connect(const handler_type& handler)
		{
			lock_guard lg(_storage.get_lock_primitive());

			life_assurance la;
			auto id = _storage.add_handler(handler, la.get_life_checker());
			return token::create<connection>(std::move(la), storage_ref(_storage), id);
		}

		template < typename... Args >
		void operator() (Args&&... args) const
		{
			handlers_stack_container handlers_copy;

			{
				lock_guard lg(_storage.get_lock_primitive());
				handlers_copy.assign(_storage.get_handlers().begin(), _storage.get_handlers().end());
			}

			for (const auto& h : handlers_copy)
			{
				execution_guard g(h.get_life_checker());
				if (g.is_alive())
					h.get_handler()(std::forward<Args>(args)...);
			}
		}
	};


	template < typename Signature_ >
	using signal = basic_signal<Signature_, signal_policies::threading::own_mutex<std::mutex>, signal_policies::handlers_storage::shared_list, signal_policies::life_assurance::life_tokens>;

}

#endif
