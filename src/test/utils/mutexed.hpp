#ifndef UTILS_MUTEXED_HPP
#define UTILS_MUTEXED_HPP

// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <thread>

#include <test/utils/thread.hpp>


namespace wigwag
{

    template < typename T_ >
    class mutexed
    {
    private:
        mutable std::mutex      _m;
        T_                      _val;

    public:
        mutexed(T_ val = T_())
            : _val(val)
        { }

        T_ get() const
        {
            auto l = lock(_m);
            return _val;
        }

        void set(T_ val)
        {
            auto l = lock(_m);
            _val = val;
        }
    };

}

#endif
