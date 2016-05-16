#ifndef WIGWAG_HANDLER_ATTRIBUTES_HPP
#define WIGWAG_HANDLER_ATTRIBUTES_HPP

// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <wigwag/detail/flags.hpp>

#include <stdint.h>


namespace wigwag
{

#include <wigwag/detail/disable_warnings.hpp>

	enum class handler_attributes : uint8_t
	{
		none					= 0x0,
		suppress_populator		= 0x1
	};

	WIGWAG_DECLARE_ENUM_BITWISE_OPERATORS(handler_attributes)

#include <wigwag/detail/enable_warnings.hpp>

}

#endif
