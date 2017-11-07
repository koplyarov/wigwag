#ifndef WIGWAG_TOKEN_HPP
#define WIGWAG_TOKEN_HPP

// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <memory>


namespace wigwag
{

#include <wigwag/detail/disable_warnings.hpp>

    class token
    {
    public:
        struct implementation
        {
            virtual void release_token_impl() = 0;
            virtual ~implementation() { }
        };

    private:
        implementation*     _impl;

    private:
        token(implementation* impl)
            : _impl(impl)
        { }

    public:
        token()
            : _impl(nullptr)
        { }

        token(token&& other)
            : _impl(other._impl)
        { other._impl = nullptr; }

        ~token()
        { reset(); }

        token(const token&) = delete;
        token& operator = (const token&) = delete;

        token& operator = (token&& other)
        {
            reset();

            _impl = other._impl;
            other._impl = nullptr;

            return *this;
        }

        void reset()
        {
            if (!_impl)
                return;

            _impl->release_token_impl();
            _impl = nullptr;
        }

        template < typename Implementation_, typename... Args_ >
        static token create(Args_&&... args)
        { return token(new Implementation_(std::forward<Args_>(args)...)); }
    };

#include <wigwag/detail/enable_warnings.hpp>

}

#endif
