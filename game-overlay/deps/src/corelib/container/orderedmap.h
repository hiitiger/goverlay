#pragma once

namespace Storm
{
    template<class K, class V>
    class OrderedMap
    {
        std::vector<V> valueVec_;
        std::unordered_map<K, int> indexMap_;;
    public:
        typedef ptrdiff_t difference_type;
        typedef V value_type;
        typedef V* pointer;
        typedef V& reference;

        OrderedMap() = default;
        OrderedMap(const OrderedMap&) = default;
        OrderedMap(OrderedMap&&) = default;
        OrderedMap& operator=(const OrderedMap&) = default;
        OrderedMap& operator=(OrderedMap&&) = default;

        int size() const;
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

        std::vector<V>  values() const;
    };


    template<class K, class V>
    int OrderedMap<K, V>::size() const
    {
        return valueVec_.size();
    }

    template<class K, class V>
    void OrderedMap<K, V>::reserve(int sz)
    {
        indexMap_.reserve(sz);
        valueVec_.reserve(sz);
    }

    template<class K, class V>
    int OrderedMap<K, V>::indexOf(const K& key) const
    {
        return indexMap_[key];
    }

    template<class K, class V>
    bool OrderedMap<K, V>::contains(const K& key) const
    {
        return indexMap_.find(key) != indexMap_.end();
    }

    template<class K, class V>
    const V OrderedMap<K, V>::value(const K& key) const
    {
        auto it = indexMap_.find(key);
        if (it != indexMap_.end())
        {
            return valueVec_.at(it->second);
        }
        else
            return V();
    }


    template<class K, class V>
    const K OrderedMap<K, V>::key(const V& value) const
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

    template<class K, class V>
    const K& OrderedMap<K, V>::keyAt(int index) const
    {
        auto it = std::find_if(indexMap_.begin(), indexMap_.end(), [index](const std::pair<K, int>& p) { p.second == index; });
        return it->first;
    }

    template<class K, class V>
    const V& OrderedMap<K, V>::valueAt(int index) const
    {
        return valueVec_[index];
    }

    template<class K, class V>
    V& OrderedMap<K, V>::valueAt(int index)
    {
        return valueVec_[index];
    }

    template<class K, class V>
    const V& OrderedMap<K, V>::operator[](const K& key) const
    {
        return valueVec_[this->indexMap_[key]];
    }

    template<class K, class V>
    V& OrderedMap<K, V>::operator[](const K& key)
    {
        return valueVec_[this->indexMap_[key]];
    }

    template<class K, class V>
    void OrderedMap<K, V>::insert(const K& key, const V& value)
    {
        auto it = indexMap_.find(key);
        if (it != indexMap_.end())
        {
            const int index = it->second;
            valueVec_.erase(valueVec_.begin() + index);

            valueVec_.push_back(value);
            it->second = valueVec_.size() - 1;

            for (auto& keyel : indexMap_)
            {
                if (keyel.second > index)
                {
                    keyel.second -= 1;
                }
            }
        }
        else
        {
            valueVec_.push_back(value);
            indexMap_.insert(std::make_pair(key, valueVec_.size() - 1));
        }
    }

    template<class K, class V>
    void OrderedMap<K, V>::remove(const K& key)
    {
        auto it = indexMap_.find(key);
        if (it != indexMap_.end())
        {
            const int index = it->second;
            valueVec_.erase(valueVec_.begin() + index);
            indexMap_.erase(it);

            for (auto& keyel : indexMap_)
            {
                if (keyel.second > index)
                {
                    keyel.second -= 1;
                }
            }
        }
    }


    template<class K, class V>
    std::vector<V> Storm::OrderedMap<K, V>::values() const
    {
        return valueVec_;
    }
}