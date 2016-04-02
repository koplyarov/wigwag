#ifndef WIGWAG_DETAIL_POLICY_PICKER_HPP
#define WIGWAG_DETAIL_POLICY_PICKER_HPP

// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <type_traits>


namespace wigwag {
namespace detail
{

	template < template <typename> class PolicyConcept_, typename DefaultPolicy_ >
	struct policies_config_entry
	{
		template < typename T_ > using concept = PolicyConcept_<T_>;
		using default_policy = DefaultPolicy_;
	};


	template < typename... Entries_ >
	struct policies_config
	{
		template < typename Policy_ >
		using policy_supported = std::false_type;

		template < template <typename> class Concept_ >
		using default_policy = void;
	};

	template < typename EntriesHead_, typename... Entries_ >
	struct policies_config<EntriesHead_, Entries_...>
	{
		template < typename Policy_ >
		using policy_supported = typename std::conditional<
				!std::is_same<typename EntriesHead_::template concept<Policy_>::adapted_policy, void>::value,
				std::true_type,
				typename policies_config<Entries_...>::template policy_supported<Policy_>
			>::type;

		template < template <typename> class Concept_ >
		using default_policy = typename std::conditional<
				std::is_same<typename EntriesHead_::template concept<int>, Concept_<int>>::value,
				typename EntriesHead_::default_policy,
				typename policies_config<Entries_...>::template default_policy<Concept_>
			>::type;
	};


	template <
		template <typename> class PolicyConcept_,
		typename PoliciesConfig_,
		typename... Policies_ >
	struct policy_picker
	{ using type = typename PoliciesConfig_::template default_policy<PolicyConcept_>; };

	template <
		template <typename> class PolicyConcept_,
		typename PoliciesConfig_,
		typename PoliciesHead_,
		typename... Policies_ >
	struct policy_picker<PolicyConcept_, PoliciesConfig_, PoliciesHead_, Policies_...>
	{
		static_assert(PoliciesConfig_::template policy_supported<PoliciesHead_>::value, "Unexpected policy!");

		using type = typename std::conditional<
			!std::is_same<typename PolicyConcept_<PoliciesHead_>::adapted_policy, void>::value,
			typename PolicyConcept_<PoliciesHead_>::adapted_policy,
			typename policy_picker<PolicyConcept_, PoliciesConfig_, Policies_...>::type>::type;
	};


}}

#endif
