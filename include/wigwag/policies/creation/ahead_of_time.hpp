#ifndef WIGWAG_POLICIES_CREATION_AHEAD_OF_TIME_HPP
#define WIGWAG_POLICIES_CREATION_AHEAD_OF_TIME_HPP

// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <wigwag/policies/creation/tag.hpp>

#include <utility>


namespace wigwag {
namespace creation
{

#include <wigwag/detail/disable_warnings.hpp>

    struct ahead_of_time
    {
        using tag = creation::tag<api_version<2, 0>>;

        template < typename OwningPtr_, typename DefaultType_ >
        class storage
        {
        private:
            OwningPtr_      _ptr;

        public:
            template < typename T_, typename... Args_ >
            void create(Args_&&... args)
            { _ptr.reset(new T_(std::forward<Args_>(args)...)); }

            const OwningPtr_& get_ptr() const
            { return _ptr; }

            bool constructed() const
            { return (bool)_ptr; }
        };
    };

#include <wigwag/detail/enable_warnings.hpp>

}}

#endif
