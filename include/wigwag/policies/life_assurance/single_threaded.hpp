#ifndef WIGWAG_POLICIES_LIFE_ASSURANCE_SINGLE_THREADED_HPP
#define WIGWAG_POLICIES_LIFE_ASSURANCE_SINGLE_THREADED_HPP

// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <wigwag/detail/config.hpp>
#include <wigwag/detail/intrusive_ptr.hpp>
#include <wigwag/policies/life_assurance/tag.hpp>


namespace wigwag {
namespace life_assurance
{

#include <wigwag/detail/disable_warnings.hpp>

    struct single_threaded
    {
        using tag = life_assurance::tag<api_version<2, 0>>;

        class life_checker;
        class execution_guard;

        class shared_data
        { };

        struct life_assurance
        {
            friend class life_checker;
            friend class execution_guard;

        private:
            mutable bool    _alive;
            mutable int     _ref_count;

        public:
            life_assurance()
                : _alive(true), _ref_count(2) // One ref in signal, another in token
            { }

            virtual ~life_assurance()
            { }

            life_assurance(const life_assurance&) = delete;
            life_assurance& operator = (const life_assurance&) = delete;


            void add_ref() const
            { ++_ref_count; }

            void release() const
            {
                if (release_node())
                    delete this;
            }

            void release_life_assurance(const shared_data&)
            { _alive = false; }

            bool node_should_be_released() const
            { return _ref_count == 1; }

            bool release_node() const
            { return --_ref_count == 0; }
        };

        class life_checker
        {
            friend class execution_guard;

            wigwag::detail::intrusive_ptr<const life_assurance>     _la;

        public:
            life_checker(const shared_data&, const life_assurance& la) WIGWAG_NOEXCEPT
                : _la(&la)
            { la.add_ref(); }
        };

        class execution_guard
        {
            wigwag::detail::intrusive_ptr<const life_assurance>     _la;

        public:
            execution_guard(const life_checker& c)
                : _la(c._la)
            { }

            execution_guard(const shared_data&, const life_assurance& la)
                : _la(&la)
            { la.add_ref(); }

            ~execution_guard()
            { }

            int is_alive() const WIGWAG_NOEXCEPT
            { return _la->_alive; }
        };
    };

#include <wigwag/detail/enable_warnings.hpp>

}}

#endif
