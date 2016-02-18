#ifndef WIGWAG_DETAIL_INTRUSIVE_REF_COUNTER_HPP
#define WIGWAG_DETAIL_INTRUSIVE_REF_COUNTER_HPP

// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <wigwag/detail/annotations.hpp>

#include <atomic>


namespace wigwag {
namespace detail
{

	template < typename Derived_ >
	class intrusive_ref_counter
	{
	private:
		std::atomic<int>	_ref_count;

	public:
		intrusive_ref_counter()
			: _ref_count(1)
		{ }

		intrusive_ref_counter(const intrusive_ref_counter&) = delete;
		intrusive_ref_counter& operator = (const intrusive_ref_counter&) = delete;

		void add_ref()
		{ ++_ref_count; }

		void release()
		{
			if (--_ref_count == 0)
			{
				WIGWAG_ANNOTATE_HAPPENS_AFTER(this);
				WIGWAG_ANNOTATE_RELEASE(this);

				delete static_cast<Derived_*>(this);
			}
			else
				WIGWAG_ANNOTATE_HAPPENS_BEFORE(this);
		}
	};

}}

#endif
