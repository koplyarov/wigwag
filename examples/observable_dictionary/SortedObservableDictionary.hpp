#ifndef EXAMPLES_OBSERVABLE_DICTIONARY_SORTEDOBSERVABLEDICTIONARY_HPP
#define EXAMPLES_OBSERVABLE_DICTIONARY_SORTEDOBSERVABLEDICTIONARY_HPP

// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <map>

#include "../common/Signals.hpp"
#include "IObservableDictionary.hpp"


template < typename Key_, typename Value_ >
class SortedObservableDictionary : public virtual IObservableDictionary<Key_, Value_>
{
    using Mutex = std::recursive_mutex;
    using Lock = std::lock_guard<Mutex>;
    using Listener = IObservableDictionaryListener<Key_, Value_>;
    using ListenerPtr = std::shared_ptr<Listener>;

private:
    std::map<Key_, Value_>                                              _map;
    std::shared_ptr<Mutex>                                              _mutex;
    SharedRMutexSignal<void(CollectionOp, const Key_&, const Value_&)>  _onChanged;
    SharedRMutexListenable<ListenerPtr>                                 _listenable;

public:
    SortedObservableDictionary()
        : _mutex(std::make_shared<Mutex>()),
          _onChanged(_mutex, [&](const typename decltype(_onChanged)::handler_type& h) { for (auto p : _map) h(CollectionOp::ItemAdded, p.first, p.second); }),
          _listenable(_mutex, [&](const ListenerPtr& l) { for (auto p : _map) l->OnItemAdded(p.first, p.second); })
    { }

    virtual std::recursive_mutex& SyncRoot() const
    { return *_mutex; }

    virtual wigwag::signal_connector<void(CollectionOp, const Key_&, const Value_&)> OnChanged() const
    { return _onChanged.connector(); }

    virtual wigwag::token AddListener(const IObservableDictionaryListenerPtr<Key_, Value_>& listener) const
    { return _listenable.connect(listener); }

    virtual int GetCount() const
    {
        Lock l(*_mutex);
        return _map.size();
    }

    virtual bool IsEmpty() const
    {
        Lock l(*_mutex);
        return _map.empty();
    }

    virtual Value_ Get(const Key_& key) const
    {
        Lock l(*_mutex);
        auto it = _map.find(key);
        if (it == _map.end())
            throw std::runtime_error("Key not found!");
        return it->second;
    }

    virtual bool ContainsKey(const Key_& key) const
    {
        Lock l(*_mutex);
        return _map.find(key) != _map.end();
    }

    virtual bool TryGet(const Key_& key, Value_& outValue) const
    {
        Lock l(*_mutex);
        auto it = _map.find(key);
        if (it == _map.end())
            return false;
        outValue = it->second;
        return true;
    }

    virtual void Set(const Key_& key, const Value_& value)
    {
        Lock l(*_mutex);
        auto it = _map.find(key);
        if (it != _map.end())
        {
            _onChanged(CollectionOp::ItemRemoved, key, it->second);
            _listenable.invoke([&](const ListenerPtr& l) { l->OnItemRemoved(key, it->second); });
            it->second = value;
        }
        else
            _map.insert({key, value});

        _onChanged(CollectionOp::ItemAdded, key, value);
        _listenable.invoke([&](const ListenerPtr& l) { l->OnItemAdded(key, value); });
    }

    virtual void Remove(const Key_& key)
    {
        if (!TryRemove(key))
            throw std::runtime_error("Key not found!");
    }

    virtual bool TryRemove(const Key_& key)
    {
        Lock l(*_mutex);
        auto it = _map.find(key);
        if (it == _map.end())
            return false;

        _onChanged(CollectionOp::ItemRemoved, key, it->second);
        _listenable.invoke([&](const ListenerPtr& l) { l->OnItemRemoved(key, it->second); });
        _map.erase(it);
        return true;
    }

    virtual void Clear()
    {
        Lock l(*_mutex);
        for (auto p : _map)
        {
            _onChanged(CollectionOp::ItemRemoved, p.first, p.second);
            _listenable.invoke([&](const ListenerPtr& l) { l->OnItemRemoved(p.first, p.second); });
        }
        _map.clear();
    }
};

#endif
