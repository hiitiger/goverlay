#pragma once

namespace Storm
{
    template<class T>
    class TinySet
    {
        Array<T> valueArray_;
    public:
        TinySet() = default;
        TinySet(TinySet&& other) = default;
        TinySet(const TinySet& other) = default;
        TinySet& operator=(TinySet&& other) = default;
        TinySet& operator=(const TinySet& other) = default;

        int size() const { return valueArray_.size(); }
        int capacity() const { return valueArray_.capacity(); }
        void reserve(int sz) { valueArray_.reserve(sz); }

        const T& at(int index) const
        {
            return valueArray_.at(index);
        }

        T& at(int index)
        {
            return valueArray_.at(index);
        }

        int indexOf(const T& value) const
        {
            const T* p = std::lower_bound(valueArray_.begin(), valueArray_.end(), value);
            if (p != valueArray_.end() && *p == value)
            {
                return int(p - valueArray_.begin());
            }
            else
            {
                return -1;
            }
        }

        bool contains(const T& value) const
        {
            return indexOf(value) != -1;
        }

        void insert(const T& value)
        {
            T* p = std::lower_bound(valueArray_.begin(), valueArray_.end(), value);
            if ((p != valueArray_.end()) && (*p == value))
            {
                *p = value;
            }
            else
            {
                int index = int(p - valueArray_.begin());
                valueArray_.insert(index, value);
            }
        }

        void remove(const T& value)
        {
            int index = indexOf(value);
            if (index != -1)
            {
                valueArray_.erase(index);
            }
        }

        const  T* begin() const { return valueArray_.begin(); }
        T* begin() { return valueArray_.begin(); }
        const  T* end() const { return valueArray_.end(); }
        T* end() { return valueArray_.end(); }

        const T* find(const T& value) const
        {
            const T* p = std::lower_bound(valueArray_.begin(), valueArray_.end(), value);
            if ((p != valueArray_.end()) && (*p == value))
                return p;
            else
                return end();
        }

        T* find(const T& value)
        {
            T* p = std::lower_bound(valueArray_.begin(), valueArray_.end(), value);
            if ((p != valueArray_.end()) && (*p == value))
                return p;
            else
                return end();
        }
    };
}