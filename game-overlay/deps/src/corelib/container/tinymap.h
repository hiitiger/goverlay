#pragma once

namespace Storm
{
    template <class K, class V >
    class TinyMap
    {
        Array<std::pair<K, V>> keyvalueArray_;
    public:
        typedef ptrdiff_t difference_type;
        typedef std::pair<K, V> value_type;
        typedef std::pair<K, V>* pointer;
        typedef std::pair<K, V>& reference;

        TinyMap() = default;
        TinyMap(TinyMap&& other) = default;
        TinyMap(const TinyMap& other) = default;
        TinyMap& operator=(TinyMap&& other) = default;
        TinyMap& operator=(const TinyMap& other) = default;

        int size() const;
        int capacity() const;
        void reserve(int sz);

        int indexOf(const K& key) const;  
        bool contains(const K& key) const;
  
        const V value(const K& key) const;
        const K key(const V& value) const;
  
        const K& keyAt(int index) const;
        const V& valueAt(int index) const;
        V& valueAt(int index);

        const V& operator[](const K& key) const;
        V& operator[](const K& key);

        void insert(const K& key, const V& value);
        void remove(const K& key);

        const  std::pair<K, V>* begin() const;
        std::pair<K, V>* begin();

        const  std::pair<K, V>* end() const;
        std::pair<K, V>* end();

        const std::pair<K, V>* find(const K&) const;
        std::pair<K, V>* find(const K&);
    };

    template <class K, class V >
    int TinyMap<K, V>::size() const
    {
        return keyvalueArray_.size();
    }

    template <class K, class V >
    int TinyMap<K, V>::capacity() const
    {
        return keyvalueArray_.capacity();
    }

    template <class K, class V >
    void TinyMap<K, V>::reserve(int sz)
    {
        keyvalueArray_.reserve(sz);
    }

    template <class K, class V >
    int TinyMap<K, V>::indexOf(const K& key) const
    {
        const std::pair<K, V>* p = std::lower_bound(keyvalueArray_.begin(), keyvalueArray_.end(), key, [](const value_type& i, const K& key) { return i.first < key ; });
        if (p != keyvalueArray_.end() && p->first == key)
        {
            return int(p - keyvalueArray_.begin());
        }
        else
        {
            return -1;
        }
    }

    template <class K, class V >
    bool TinyMap<K, V>::contains(const K& key) const
    {
        return std::binary_search(keyvalueArray_.begin(), keyvalueArray_.end(), key, [](const value_type& i, const K& key) { return i.first == key; });
    }

    template <class K, class V >
    const V TinyMap<K, V>::value(const K& key) const
    {
        int index = indexOf(key);
        if (index != -1)
        {
            return keyvalueArray_.at(index).value();
        }
        return V();
    }

    template <class K, class V >
    const K TinyMap<K, V>::key(const V& value) const
    {
        for (int i = 0; i != size(); ++i)
        {
            if (valueAt(i) == value)
            {
                return keyAt(i);
            }
        }
        return K();
    }

    template <class K, class V >
    const K& TinyMap<K, V>::keyAt(int index) const
    {
        return keyvalueArray_[index].first;
    }

    template <class K, class V >
    const V& TinyMap<K, V>::valueAt(int index) const
    {
        return keyvalueArray_[index].second;
    }

    template <class K, class V >
    V& TinyMap<K, V>::valueAt(int index)
    {
        return keyvalueArray_[index].second;
    }

    template <class K, class V >
    const V& TinyMap<K, V>::operator[](const K& key) const
    {
        int index = indexOf(key);
        return keyvalueArray_.at(index).second;
    }

    template <class K, class V >
    V& TinyMap<K, V>::operator[](const K& key)
    {
        int index = indexOf(key);
        return keyvalueArray_.at(index).second;
    }

    template <class K, class V >
    void TinyMap<K, V>::insert(const K& key, const V& value)
    {
        std::pair<K, V>* p = std::lower_bound(keyvalueArray_.begin(), keyvalueArray_.end(), key, [](value_type& i, const K& key) { return i.first < key; });
        if (p != keyvalueArray_.end() && p->first == key)
        {
            *p = std::pair<K, V>(key, value);
        }
        else
        {
            int index = int(p - keyvalueArray_.begin());
            keyvalueArray_.insert(index, std::pair<K, V>(key, value));
        }
    }

    template <class K, class V >
    void TinyMap<K, V>::remove(const K& key)
    {
        int index = indexOf(key);
        if (index != -1)
        {
            keyvalueArray_.erase(index);
        }
    }

    template <class K, class V >
    const std::pair<K, V>* TinyMap<K, V>::begin() const
    {
        return keyvalueArray_.begin();
    }

    template <class K, class V >
    std::pair<K, V>* TinyMap<K, V>::begin()
    {
        return keyvalueArray_.begin();
    }

    template <class K, class V >
    const std::pair<K, V>* TinyMap<K, V>::end() const
    {
        return keyvalueArray_.end();
    }

    template <class K, class V >
    std::pair<K, V>* TinyMap<K, V>::end()
    {
        return keyvalueArray_.end();
    }

    template <class K, class V >
    const std::pair<K, V>* TinyMap<K, V>::find(const K& key) const
    {
        const std::pair<K, V>* p = std::lower_bound(keyvalueArray_.begin(), keyvalueArray_.end(), key, [](const value_type& i, const K& key) { return i.first < key; });
        if (p != keyvalueArray_.end() && p->first == key)
        {
            return p;
        }
        else
            return end();
    }


    template <class K, class V >
    std::pair<K, V>* TinyMap<K, V>::find(const K& key)
    {
        std::pair<K, V>* p = std::lower_bound(keyvalueArray_.begin(), keyvalueArray_.end(), key, [](value_type& i, const K& key) { return i.first < key; });
        if (p != keyvalueArray_.end() && p->first == key)
        {
            return p;
        }
        else
            return end();
    }

}