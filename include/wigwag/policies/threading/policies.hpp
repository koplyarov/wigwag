#ifndef WIGWAG_POLICIES_THREADING_POLICIES_HPP
#define WIGWAG_POLICIES_THREADING_POLICIES_HPP

// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <wigwag/policies/threading/none.hpp>
#include <wigwag/policies/threading/own_mutex.hpp>
#include <wigwag/policies/threading/own_recursive_mutex.hpp>
#include <wigwag/policies/threading/shared_mutex.hpp>
#include <wigwag/policies/threading/shared_recursive_mutex.hpp>

namespace wigwag {
namespace threading
{

    using default_ = own_recursive_mutex;

}}

#endif
