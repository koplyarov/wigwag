#ifndef WIGWAG_DETAIL_INTRUSIVE_LIST_HPP
#define WIGWAG_DETAIL_INTRUSIVE_LIST_HPP

// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <wigwag/detail/iterator_base.hpp>

#include <iterator>
#include <type_traits>


namespace wigwag {
namespace detail
{

	template < typename T_ >
	class intrusive_list;


	class intrusive_list_node
	{
		template < typename T_ >
		friend class intrusive_list;

	private:
		intrusive_list_node*	_prev;
		intrusive_list_node*	_next;

	public:
		intrusive_list_node() : _prev(this), _next(this) { }

	private:
		void insert_before(intrusive_list_node& other)
		{
			_prev = other._prev;
			_next = &other;

			_next->_prev = this;
			_prev->_next = this;
		}

		void unlink()
		{
			_next->_prev = _prev;
			_prev->_next = _next;
			_prev = _next = this;
		}

		bool unlinked() const
		{ return _next == this && _prev == this; }
	};


	template < typename T_ >
	class intrusive_list
	{
		static_assert(std::is_base_of<intrusive_list_node, T_>::value, "intrusive_list_node should be a base of T_");

	public:
		class iterator : public iterator_base<iterator, std::bidirectional_iterator_tag, T_>
		{
		private:
			intrusive_list_node*	_node;

		public:
			explicit iterator(intrusive_list_node* node = nullptr) : _node(node) { }

			T_& dereference() const { return static_cast<T_&>(*_node); }
			bool equal(iterator other) const { return _node == other._node; }
			void increment() { _node = _node->_next; }
			void decrement() { _node = _node->_prev; }
		};

		class const_iterator : public iterator_base<const_iterator, std::bidirectional_iterator_tag, const T_>
		{
		private:
			const intrusive_list_node*	_node;

		public:
			explicit const_iterator(const intrusive_list_node* node = nullptr) : _node(node) { }

			const T_& dereference() const { return static_cast<const T_&>(*_node); }
			bool equal(const_iterator other) const { return _node == other._node; }
			void increment() { _node = _node->_next; }
			void decrement() { _node = _node->_prev; }
		};

	private:
		intrusive_list_node		_root;

	public:
		iterator begin() { return iterator(_root._next); }
		iterator end() { return iterator(&_root); }

		const_iterator begin() const { return const_iterator(_root._next); }
		const_iterator end() const { return const_iterator(&_root); }

		bool empty() const { return _root.unlinked(); }
		size_t size() const { return std::distance(begin(), end()); }

		void push_back(T_& node) { node.insert_before(_root); }
		void erase(T_& node) { node.unlink(); }
	};

}}

#endif
