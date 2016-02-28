#ifndef EXAMPLES_OBSERVABLE_DICTIONARY_IOBSERVABLEDICTIONARY_HPP
#define EXAMPLES_OBSERVABLE_DICTIONARY_IOBSERVABLEDICTIONARY_HPP

// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <wigwag/listenable.hpp>
#include <wigwag/signal.hpp>

#include "CollectionOp.hpp"


template < typename Key_, typename Value_ >
struct IObservableDictionaryListener
{
	virtual ~IObservableDictionaryListener() { }

	virtual void OnItemAdded(const Key_&, const Value_&) = 0;
	virtual void OnItemRemoved(const Key_&, const Value_&) = 0;
};

template < typename Key_, typename Value_ >
using IObservableDictionaryListenerPtr = std::shared_ptr<IObservableDictionaryListener<Key_, Value_>>;


template < typename Key_, typename Value_ >
struct IObservableDictionary
{
	virtual ~IObservableDictionary() { }

	virtual std::recursive_mutex& SyncRoot() const = 0;

	virtual wigwag::signal_connector<void(CollectionOp, const Key_&, const Value_&)> OnChanged() const = 0;
	virtual wigwag::token AddListener(const IObservableDictionaryListenerPtr<Key_, Value_>& listener) const = 0;

	virtual int GetCount() const = 0;
	virtual bool IsEmpty() const = 0;

	virtual Value_ Get(const Key_& key) const = 0;
	virtual bool ContainsKey(const Key_& key) const = 0;
	virtual bool TryGet(const Key_& key, Value_& outValue) const = 0;

	virtual void Set(const Key_& key, const Value_& value) = 0;
	virtual void Remove(const Key_& key) = 0;
	virtual bool TryRemove(const Key_& key) = 0;

	virtual void Clear() = 0;
};


template < typename Key_, typename Value_ >
using IObservableDictionaryPtr = std::shared_ptr<IObservableDictionary<Key_, Value_>>;

#endif
