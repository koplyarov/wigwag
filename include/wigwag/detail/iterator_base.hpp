#ifndef WIGWAG_DETAIL_ITERATOR_BASE_HPP
#define WIGWAG_DETAIL_ITERATOR_BASE_HPP

// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <iterator>
#include <type_traits>


namespace wigwag {
namespace detail
{

#include <wigwag/detail/disable_warnings.hpp>

    template <
            typename Derived_,
            typename Category_,
            typename T_,
            typename Distance_ = std::ptrdiff_t,
            typename Pointer_ = T_*,
            typename Reference_ = T_&
        >
    class iterator_base : public std::iterator<Category_, T_, Distance_, Pointer_, Reference_>
    {
    public:
        bool operator == (Derived_ other) const { return get_derived().equal(other); }
        bool operator != (Derived_ other) const { return !(*this == other); }

        bool operator < (Derived_ other) const { return get_derived().distance_to(other) < 0; }
        bool operator > (Derived_ other) const { return get_derived().distance_to(other) > 0; }
        bool operator <= (Derived_ other) const { return !(other < *this); }
        bool operator >= (Derived_ other) const { return !(other > *this); }

        Reference_ operator * () { return get_derived().dereference(); }
        const Reference_ operator * () const { return get_derived().dereference(); }

        Pointer_ operator -> () { return &(get_derived().dereference()); }
        const Pointer_ operator -> () const { return &(get_derived().dereference()); }

        Derived_& operator ++ () { get_derived().increment(); return get_derived(); }
        Derived_ operator ++ (int) { Derived_ tmp(get_derived()); ++*this; return tmp; }

        Derived_& operator -- () { get_derived().decrement(); return get_derived(); }
        Derived_ operator -- (int) { Derived_ tmp(get_derived()); --*this; return tmp; }

        Derived_& operator += (Distance_ n) { get_derived().advance(n); return get_derived(); }
        Derived_& operator -= (Distance_ n) { get_derived().advance(-n); return get_derived(); }

        Distance_ operator - (Derived_ other) const { return other.distance_to(get_derived()); }

        Derived_ operator + (Distance_ n) const { Derived_ tmp(get_derived()); return tmp += n; }
        Derived_ operator - (Distance_ n) const { Derived_ tmp(get_derived()); return tmp -= n; }

        Reference_ operator[] (Distance_ n) const { Derived_ tmp(get_derived()); tmp.advance(n); return tmp.dereference(); }

    protected:
        Derived_& get_derived() { return static_cast<Derived_&>(*this); }
        const Derived_& get_derived() const { return static_cast<const Derived_&>(*this); }
    };


    template < typename Derived_, typename Category_, typename T_, typename Distance_, typename Pointer_, typename Reference_ >
    Derived_ operator + (Distance_ n, iterator_base<Derived_, Category_, T_, Distance_, Pointer_, Reference_> it)
    { return it + n; }


#include <wigwag/detail/enable_warnings.hpp>

}}

#endif
