#ifndef WIGWAG_DETAIL_STORAGE_FOR_HPP
#define WIGWAG_DETAIL_STORAGE_FOR_HPP

// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <wigwag/detail/config.hpp>

#include <type_traits>
#include <utility>


namespace wigwag {
namespace detail
{

#include <wigwag/detail/disable_warnings.hpp>

#if WIGWAG_HAS_UNRESTRICTED_UNIONS
    template < typename T_ >
    union storage_for
    {
    public:
        struct no_construct_tag { };

    private:
        T_ obj;

    public:
        storage_for(no_construct_tag) { }

        template < typename... Args_ >
        storage_for(Args_&&... args) : obj(std::forward<Args_>(args)...) { }
        ~storage_for() { }

        template < typename... Args_ >
        void construct(Args_&&... args)
        { new(&obj) T_(std::forward<Args_>(args)...); }

        void destruct()
        { obj.~T_(); }

        T_& ref() { return obj; }
        const T_& ref() const { return obj; }
    };
#else
    template < typename T_ >
    class storage_for
    {
    public:
        struct no_construct_tag { };

    private:
        using storage = typename std::aligned_storage<sizeof(T_), WIGWAG_ALIGNOF(T_)>::type;

    private:
        storage     obj;

    public:
        storage_for(no_construct_tag) { }

        template < typename... Args_ >
        storage_for(Args_&&... args) { new(&obj) T_(std::forward<Args_>(args)...); }
        ~storage_for() { }

        template < typename... Args_ >
        void construct(Args_&&... args)
        { new(&obj) T_(std::forward<Args_>(args)...); }

        void destruct()
        { obj.~T_(); }

        T_& ref() { return *reinterpret_cast<T_*>(&obj); }
        const T_& ref() const { return *reinterpret_cast<const T_*>(&obj); }
    };
#endif

#include <wigwag/detail/enable_warnings.hpp>

}}

#endif
