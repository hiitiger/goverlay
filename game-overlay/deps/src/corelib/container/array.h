#pragma once
namespace Storm
{
    template<class T>
    struct ArrayData : CowData
    {
        ArrayData& operator=(const ArrayData&) = delete;
        static constexpr int minCap_ = 8;
        static constexpr int minGrow_ = 8;

        T* array_ = nullptr;
        T* arrayEnd_ = nullptr;
        T* begin_ = nullptr;
        T* end_ = nullptr;

        ArrayData() = default;
        ~ArrayData()
        {
            destroy();
        }

        ArrayData(const ArrayData& other)
        {
            reserve(other.size());
            copyConstruct(begin_, other.begin_, other.size());
            end_ = begin_ + other.size();
        }

        ArrayData(ArrayData&& other)
            : array_(other.array_)
            , arrayEnd_(other.arrayEnd_)
            , begin_(other.begin_)
            , end_(other.end_)

        {
            other.array_ = nullptr;
            other.arrayEnd_ = nullptr;
            other.begin_ = nullptr;
            other.end_ = nullptr;
        }

        void copyConstruct(T* dst, const T* src, int count)
        {
            for (int i = 0; i < count; i++)
            {
                new (dst++) T(*src++);
            }
        }

        void grow()
        {
            size_t newCapacity = std::max(minCap_, capacity() + std::max(minGrow_, capacity() / 2));
            size_t newFrontSpare = (newCapacity - size()) / 2;
            reserve(newCapacity, newFrontSpare);
        }

        int size() const { return end_ - begin_; }
        int capacity() const { return  arrayEnd_ - array_; }

        int frontSpare() const { return begin_ - array_; }
        int backSpare() const { return arrayEnd_ - end_; }
        int spare() const { return frontSpare() + backSpare(); }

        int indexOf(const T& t) const
        {
            if (size() > 0)
            {
                T* elemPtr = std::find(begin_, end_, t);
                if (elemPtr != end_)
                {
                    return elemPtr - begin_;
                }
            }

            return -1;
        }

        T& at(int index)
        {
            DAssert(index >= 0 && index < size());
            return begin_[index];
        }

        const T& at(int index) const
        {
            DAssert(index >= 0 && index < size());
            return begin_[index];
        }

        T& front()
        {
            DAssert(size() > 0);
            return *begin_;
        }

        const T& front() const
        {
            DAssert(size() > 0);
            return *begin_;
        }

        T& back()
        {
            DAssert(size() > 0);
            return *(end_ - 1);
        }

        const T& back() const
        {
            DAssert(size() > 0);
            return *(end_ - 1);
        }

        void reserve(int newCapacity, int newFrontSpare = 0)
        {
            newCapacity = std::max(minCap_, newCapacity);
            if (capacity() >= newCapacity)
            {
                return;
            }

            int curSize = this->size();
            DAssert(newFrontSpare + curSize <= newCapacity);

            T* array = (T*)malloc(sizeof(T) * newCapacity);
            T* begin = array + newFrontSpare;

            if (curSize > 0)
            {
                for (int i = 0; i != curSize; ++i)
                {
                    new (begin + i) T(std::move(begin_[i]));
                    begin_[i].~T();
                }
            }

            if (array_)
            {
                free(array_);
            }

            array_ = array;
            arrayEnd_ = array_ + newCapacity;
            begin_ = begin;
            end_ = begin_ + curSize;
        }

        void clear()
        {
            for (T* elemPtr = begin_; elemPtr != end_; ++elemPtr)
            {
                elemPtr->~T();
            }
            end_  = begin_;
        }

        void destroy()
        {
            for (T* elem = begin_; elem != end_; ++elem)
            {
                elem->~T();
            }
            if (array_)
            {
                free(array_);
            }
            array_ = nullptr;
            arrayEnd_ = nullptr;
            begin_ = end_ = nullptr;
        }

        T* moveInsertBack(int index)
        {
            DAssert(backSpare() > 0);
            DAssert(index >= 0 && index <= size());
            new (end_) T(std::move(back()));
            for (T* elemPtr = end_ - 1; elemPtr > begin_ + index; --elemPtr)
            {
                *elemPtr = std::move(*(elemPtr - 1));
            }
            end_ += 1;
            return begin_ + index;
        }

        T* moveInsertFront(int index)
        {
            DAssert(frontSpare() > 0);
            DAssert(index >= 0 && index <= size());
            T* newBegin = begin_ - 1;
            new (newBegin) T(std::move(front()));
            for (T* elemPtr = begin_; elemPtr < begin_ + index - 1; ++elemPtr)
            {
                *elemPtr = std::move(*(elemPtr + 1));
            }
            begin_ = newBegin;
            return begin_ + index;

        }

        T* getInsertPtr(int index, bool& constructed)
        {
            DAssert(index >= 0 && index <= size());

            constructed = true;

            if (index == size())
            {
                if (backSpare() > 0)
                {
                    constructed = false;
                    return end_++;
                }
                else if (frontSpare() > 0)
                {
                    T* elemPtr = moveInsertFront(index);
                    return elemPtr;
                }
                else
                {
                    grow();
                    constructed = false;
                    return end_++;
                }
            }
            else if (index == 0)
            {
                if (frontSpare() > 0)
                {
                    constructed = false;
                    return --begin_;
                }
                else if (backSpare() > 0)
                {
                    T* elemPtr = moveInsertBack(index);
                    return elemPtr;
                }
                else
                {
                    grow();
                    constructed = false;
                    return --begin_;
                }
            }

            else
            {
                if (backSpare() > 0)
                {
                    T* elemPtr = moveInsertBack(index);
                    return elemPtr;
                }
                else if (frontSpare() > 0)
                {
                    T* elemPtr = moveInsertFront(index);
                    return elemPtr;
                }
                else
                {
                    grow();
                    T* elemPtr = moveInsertBack(index);
                    return elemPtr;
                }
            }
        }

        void insert(int index, const T& t)
        {
            bool constructed = true;
            T* elemPtr = getInsertPtr(index, constructed);
            if (constructed)
            {
                *elemPtr = t;
            }
            else
            {
                new (elemPtr) T(t);
            }
        }

        void insert(int index, T&& t)
        {
            bool constructed = true;
            T* elemPtr = getInsertPtr(index, constructed);
            if (constructed)
            {
                *elemPtr = std::move(t);
            }
            else
            {
                new (elemPtr) T(std::move(t));
            }
        }

        void push_back(const T& t)
        {
            insert(size(), t);
        }

        void push_back(T&& t)
        {
            insert(size(), std::move(t));
        }

        void push_front(const T& t)
        {
            insert(0, t);
        }

        void push_front(T&& t)
        {
            insert(0, std::move(t));
        }

        void erase(int index)
        {
            if (index >= 0 && index < size())
            {
                if (index < size() / 2)
                {
                    T* elemPtr = begin_ + index;
                    elemPtr->~T();
                    for (; elemPtr != begin_; -- elemPtr)
                    {
                        *elemPtr = std::move(*(elemPtr - 1));
                    }

                    begin_ += 1;
                }
                else
                {
                    T* elemPtr = begin_ + index;
                    elemPtr->~T();
                    for (; elemPtr != (end_ - 1);  ++ elemPtr)
                    {
                        *elemPtr = std::move(*(elemPtr + 1));
                    }

                    end_ -= 1;
                }
            }
        }

        void removeOne(const T& t)
        {
            int index = indexOf(t);
            if (index != -1)
            {
                erase(index);
            }
        }

        void removeAll(const T& t)
        {
            int index = indexOf(t);
            while (index != -1)
            {
                erase(index);
                index = indexOf(t);
            }
        }
    };


    template<class T>
    class Array
    {
        using CowDataType = ArrayData<T>;
        CowPtr<CowDataType> data_;
    public:
        typedef ptrdiff_t difference_type;
        typedef T value_type;
        typedef T* pointer;
        typedef T& reference;

        Array()
            : data_(new CowDataType)
        {
        }

        Array(std::initializer_list<value_type> list)
            : data_(new CowDataType)
        {
            data_->reserve(list.size());
            for (auto it = list.begin(); it != list.end(); ++it)
            {
                push_back(*it);
            }
        }

        Array(const Array& other)
            : data_(other.data_)
        {

        }

        Array(Array&& other)
            : data_(std::move(other.data_))
        {
            other.data_ = CowPtr<CowDataType>(new CowDataType());
        }

        Array& operator= (const Array& other)
        {
            if (this != &other)
                Array(other).swap(*this);
            return *this;
        }

        Array& operator= (Array&& other)
        {
            if (this != &other)
                Array(std::move(other)).swap(*this);
            return *this;
        }

        int size() const { return data_->size(); }
        int capacity() const { return  data_->capacity(); }

        int frontSpare() const { return data_->frontSpare(); }
        int backSprare() const { return data_->backSpare(); }

        const T* begin() const { return data_->begin_; }
        const T* end() const { return data_->end_; }
        T* begin() { return data_->begin_; }
        T* end() { return data_->end_; }

        int indexOf(const T& t) const { return data_->indexOf(t); }

        T& at(int index) { return data_->at(index); }
        const T& at(int index) const { return data_->at(index); }

        T& operator[](int index) { return data_->at(index); }
        const T& operator[](int index) const { return data_->at(index); }

        T& front() { return data_->front(); }
        const T& front() const { return data_->front(); }

        T& back() { return data_->back(); }
        const T& back() const { return data_->back(); }

        void reserve(int sz)
        {
            data_->reserve(sz, data_->frontSpare());
        }

        void shrink()
        {
            if (capacity() > size())
            {
                data_ = CowPtr<CowDataType>(new CowDataType(*data_.constData()));
            }
        }

        void clear()
        {
            data_->clear();
        }

        void destroy()
        {
            data_->destroy();
        }

        void insert(int index, const T& t)
        {
            data_->insert(index, t);
        }

        void insert(int index, T&& t)
        {
            data_->insert(index, std::move(t));
        }

        void push_back(const T& t)
        {
            data_->push_back(t);
        }

        void push_back(T&& t)
        {
            data_->push_back(std::move(t));
        }

        void push_front(const T& t)
        {
            data_->push_front(t);
        }

        void push_front(T&& t)
        {
            data_->push_front(std::move(t));
        }

        void erase(int index)
        {
            data_->erase(index);
        }

        void removeOne(const T& t)
        {
            data_->removeOne(t);
        }

        void removeAll(const T& t)
        {
            data_->removeAll(t);
        }

    private:
        void swap(Array& other)
        {
            std::swap(this->data_, other.data_);
        }
    };

}