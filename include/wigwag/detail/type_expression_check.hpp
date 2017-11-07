#ifndef WIGWAG_DETAIL_TYPE_EXPRESSION_CHECK_HPP
#define WIGWAG_DETAIL_TYPE_EXPRESSION_CHECK_HPP

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

#define WIGWAG_DECLARE_TYPE_EXPRESSION_CHECK(Name_, ...) \
    template < typename T_, typename Enabler = std::true_type > \
    struct Name_ \
    { static const bool value = false; }; \
    template < typename T_ > \
    struct Name_<T_, decltype((__VA_ARGS__), std::true_type())> \
    { static const bool value = true; }

}}

#endif
