#ifndef WIGWAG_POLICIES_REF_COUNTER_ATOMIC_HPP
#define WIGWAG_POLICIES_REF_COUNTER_ATOMIC_HPP

// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <wigwag/detail/annotations.hpp>
#include <wigwag/policies/ref_counter/tag.hpp>

#include <atomic>


namespace wigwag {
namespace ref_counter
{

#include <wigwag/detail/disable_warnings.hpp>

	class atomic
	{
	public:
		using tag = ref_counter::tag<api_version<2, 0>>;

	private:
		mutable std::atomic<int>	_counter;

	public:
		atomic(int initVal)
			: _counter(initVal)
		{ }

		int add_ref() const
		{ return ++_counter; }

		int release() const
		{
			auto res = --_counter;
			if (res == 0)
			{
				WIGWAG_ANNOTATE_HAPPENS_AFTER(this);
				WIGWAG_ANNOTATE_RELEASE(this);
			}
			else
				WIGWAG_ANNOTATE_HAPPENS_BEFORE(this);
			return res;
		}
	};

#include <wigwag/detail/enable_warnings.hpp>

}}

#endif
