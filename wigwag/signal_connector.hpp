#ifndef WIGWAG_SIGNAL_CONNECTOR_HPP
#define WIGWAG_SIGNAL_CONNECTOR_HPP

// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <wigwag/detail/intrusive_ptr.hpp>
#include <wigwag/detail/signal_connector_impl.hpp>


namespace wigwag
{

	template < typename Signature_ >
	class signal_connector
	{
		using handler_type = std::function<Signature_>;

		using impl_type = detail::signal_connector_impl<Signature_>;
		using impl_type_ptr = detail::intrusive_ptr<impl_type>;

	private:
		impl_type_ptr 	_impl;

	public:
		explicit signal_connector(const impl_type_ptr& impl)
			: _impl(impl)
		{ }

		token connect(const handler_type& handler)
		{ return _impl->connect(handler); }
	};

}

#endif
