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

	template <
		template <typename> class PolicyConcept_,
		template <template <typename> class> class DefaultImplementationGetter_,
		typename... Policies_ >
	struct policy_picker
	{ using type = typename DefaultImplementationGetter_<PolicyConcept_>::type; };

	template <
		template <typename> class PolicyConcept_,
		template <template <typename> class> class DefaultImplementationGetter_,
		typename PoliciesHead_,
		typename... Policies_ >
	struct policy_picker<PolicyConcept_, DefaultImplementationGetter_, PoliciesHead_, Policies_...>
	{
		using type = typename std::conditional<
			!std::is_same<typename PolicyConcept_<PoliciesHead_>::adapted_policy, void>::value,
			typename PolicyConcept_<PoliciesHead_>::adapted_policy,
			typename policy_picker<PolicyConcept_, DefaultImplementationGetter_, Policies_...>::type>::type;
	};


}}

#endif
