#ifndef WIGWAG_DETAIL_INTRUSIVE_PTR_HPP
#define WIGWAG_DETAIL_INTRUSIVE_PTR_HPP

// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <wigwag/detail/enabler.hpp>

#include <type_traits>


namespace wigwag {
namespace detail
{

	template < typename T_ >
	class intrusive_ptr
	{
		template < typename U_ >
		friend class intrusive_ptr;

	private:
		T_*		_raw;

	public:
		explicit intrusive_ptr(T_* rawPtr = nullptr)
			: _raw(rawPtr)
		{ }

		intrusive_ptr(intrusive_ptr&& other) noexcept
			: _raw(other._raw)
		{ other._raw = nullptr; }

		template < typename U_ >
		intrusive_ptr(const intrusive_ptr<U_> other, typename std::enable_if<std::is_base_of<T_, U_>::value, enabler>::type = enabler())
			: _raw(other._raw)
		{
			if (_raw)
				_raw->add_ref();
		}

		intrusive_ptr(const intrusive_ptr& other)
			: _raw(other._raw)
		{
			if (_raw)
				_raw->add_ref();
		}

		~intrusive_ptr()
		{
			if (_raw)
				_raw->release();
		}

		intrusive_ptr& operator = (const intrusive_ptr& other)
		{
			intrusive_ptr tmp(other);
			swap(tmp);
			return *this;
		}

		bool operator == (const intrusive_ptr& other) const
		{ return other._raw == _raw; }

		bool operator != (const intrusive_ptr& other) const
		{ return !(*this == other); }

		explicit operator bool() const
		{ return _raw; }

		void reset(T_* ptr = nullptr)
		{
			intrusive_ptr tmp(ptr);
			swap(tmp);
		}

		void swap(intrusive_ptr& other)
		{ std::swap(_raw, other._raw); }

		T_* get() const
		{ return _raw; }

		T_* operator -> () const
		{ return _raw; }

		T_& operator * () const
		{ return *_raw; }
	};

}}

#endif