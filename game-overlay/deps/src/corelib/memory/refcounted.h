#pragma once

/*
    侵入式引用计数
    1. 侵入式引用计数 用在强制使用引用计数管理资源的场合
    2. 提供接口的时候(COM 类似)

    非侵入式引用计数
    1. std::shared_ptr用在可以选择资源管理方式的场合
    2. 资源不一定有明确的owner
    
    对象树
    1. 资源拥有明确的owner
*/

namespace Storm {

    struct RefCountData
    {
        std::atomic<int32_t> strongRefCount_ = 0;
        std::atomic<int32_t> weakRefCount_ = 0;

        RefCountData()
        {
            strongRefCount_ = 0;
            weakRefCount_ = 0;
        }
    };


    class RefCounted 
    {
        STORM_NONCOPYABLE(RefCounted);
        friend class RefCounter;

        RefCountData* refCount_ = nullptr;
    public:
        RefCounted() = default;

        virtual ~RefCounted() { ; }

        void addRef();

        void release();

        int32_t refCount();

    protected:

        virtual void destroy()
        {
            delete this;
        }
    };

    class RefCounter
    {

    public:
        static RefCountData* refCountData(RefCounted* obj)
        {
            if (!obj->refCount_)
                obj->refCount_ = new RefCountData;

            return obj->refCount_;
        }

        static int32_t addStrongRef(RefCounted* obj)
        {
            if (!obj->refCount_)
            {
                obj->refCount_ = new RefCountData;
                obj->refCount_->weakRefCount_ += 1;
            }

            obj->refCount_->strongRefCount_ += 1;
            return obj->refCount_->strongRefCount_;
        }

        static int32_t releaseStrongRef(RefCounted* obj)
        {
            DAssert(obj->refCount_);

            obj->refCount_->strongRefCount_ -= 1;
            if (obj->refCount_->strongRefCount_ == 0)
            {
                obj->refCount_->weakRefCount_ -= 1;
            }

            return obj->refCount_->strongRefCount_;
        }

        static int32_t addWeakRef(RefCountData* refCount)
        {
            refCount->weakRefCount_ += 1;
            return refCount->weakRefCount_;
        }

        static int32_t releaseWeakRef(RefCountData* refCount)
        {
            refCount->weakRefCount_ -= 1;
            return refCount->weakRefCount_;
        }

        static int32_t strongRefCount(RefCounted* obj)
        {
            return obj->refCount_ ? obj->refCount_->strongRefCount_.load() : 0; 
        }

        static int32_t weakRefCount(RefCounted* obj)
        {
            DAssert(obj && obj->refCount_);
            return obj->refCount_->weakRefCount_;
        }

        static void destroyRefCountData(RefCountData* data)
        {
            delete data;
        }

        template<class T>
        static void destroy(T* obj)
        {
            if (obj->refCount_ && obj->refCount_->weakRefCount_ == 0)
            {
                destroyRefCountData(obj->refCount_);
            }
            obj->destroy();
        }

    };

    inline void RefCounted::addRef()
    {
        RefCounter::addStrongRef(this);
    }
   
    inline void RefCounted::release()
    {
        if (0 == RefCounter::releaseStrongRef(this))
        {
            RefCounter::destroy(this);
        }
    }

    inline int32_t RefCounted::refCount()
    {
        return RefCounter::strongRefCount(this);
    }

    template<class T>
    class RefPtr
    {
        template<class _Ty0>
        friend class RefPtr;

        T* ptr_ = nullptr;
    public:
        RefPtr() = default;

        RefPtr(T* ptr)
            : ptr_(ptr)
        {
            if (ptr_)
            {
                RefCounter::addStrongRef(ptr_);
            }
        }

        template<class U>
        RefPtr(U* ptr)
            : ptr_(static_cast<T*>(ptr))
        {
            if (ptr_)
            {
                RefCounter::addStrongRef(ptr_);
            }
        }

        ~RefPtr()
        {
            reset();
        }

        RefPtr(const RefPtr& refptr)
            : ptr_(refptr.ptr_)
        {
            if (ptr_)
            {
                RefCounter::addStrongRef(ptr_);
            }
        }

        RefPtr(RefPtr&& refptr)
            : ptr_(refptr.ptr_)
        {
            refptr.ptr_ = nullptr;
        }

        template<class U>
        RefPtr(const RefPtr<U>& refptr)
            : ptr_(static_cast<T*>(refptr.ptr_))
        {
            if (ptr_)
            {
                RefCounter::addStrongRef(ptr_);
            }
        }

        template<class U>
        RefPtr(RefPtr<U>&& refptr)
            : ptr_(static_cast<T*>(refptr.ptr_))
        {
            refptr.ptr_ = nullptr;
        }

        RefPtr& operator = (T* ptr)
        {
            RefPtr<T>(ptr).swap(*this);

            return *this;
        }

        template<class U>
        RefPtr& operator = (U* ptr)
        {
            RefPtr<T>(ptr).swap(*this);

            return *this;
        }


        RefPtr& operator = (const RefPtr& other)
        {
            RefPtr<T>(other).swap(*this);
            return *this;
        }

        template<class U>
        RefPtr& operator = (const RefPtr<U>& other)
        {
            RefPtr<T>(other).swap(*this);
            return *this;
        }

        RefPtr& operator = (RefPtr&& other)
        {
            RefPtr<T>(std::move(other)).swap(*this);
            return *this;
        }

        template<class U>
        RefPtr& operator = (RefPtr<U>&& other)
        {
            RefPtr<T>(std::move(other)).swap(*this);
            return *this;
        }

        int32_t refCount()
        {
            if (ptr_)
            {
                return RefCounter::strongRefCount(ptr_);
            }
            return 0;
        }

        T* get() const
        {
            return ptr_;
        }

        operator T*() const
        {
            return ptr_;
        }

        T* operator->() const
        {
            return ptr_;
        }

        operator bool() const
        {
            return !!ptr_;
        }

        void reset()
        {
            if (ptr_)
            {
                if (0 == RefCounter::releaseStrongRef(ptr_))
                {
                    RefCounter::destroy(ptr_);
                }
            }

            ptr_ = nullptr;
        }

    private:
        void swap(RefPtr<T>& other)
        {
            std::swap(this->ptr_, other.ptr_);
        }
    };

    template<class T>
    inline bool operator == (const RefPtr<T>& left, const RefPtr<T>& right)
    {
        return left.get() == right.get();
    }

    template<class T>
    inline bool operator != (const RefPtr<T>& left, const RefPtr<T>& right)
    {
        return left.get() != right.get();
    }

    template<class T>
    inline bool operator < (const RefPtr<T>& left, const RefPtr<T>& right)
    {
        return left.get() < right.get();
    }

    template<class T>
    inline bool operator > (const RefPtr<T>& left, const RefPtr<T>& right)
    {
        return left.get() > right.get();
    }

    template<class T>
    inline bool operator <= (const RefPtr<T>& left, const RefPtr<T>& right)
    {
        return left.get() <= right.get();
    }

    template<class T>
    inline bool operator >= (const RefPtr<T>& left, const RefPtr<T>& right)
    {
        return left.get() >= right.get();
    }

    template<class T>
    RefPtr<T> refThis(T* t)
    {
        return RefPtr<T>(t);
    }

    template<class T, class ...A>
    RefPtr<T> makeRefPtr(A&& ...args)
    {
        return RefPtr<T>(new T(std::forward<A>(args)...));
    }


    template<class T>
    class WeakRefPtr
    {
        template<class _Ty0>
        friend class WeakRefPtr;

        T* ptr_ = nullptr;
        RefCountData* refCount_ = nullptr;

    public:
        WeakRefPtr() = default;

        WeakRefPtr(T* ptr)
            : ptr_(ptr)
        {
            if (ptr_)
            {
                refCount_ = RefCounter::refCountData(ptr_);
                RefCounter::addWeakRef(refCount_);
            }
        }

        template<class U>
        WeakRefPtr(U* ptr)
            : ptr_(static_cast<T*>(ptr))
        {
            if (ptr_)
            {
                refCount_ = RefCounter::refCountData(ptr_);
                RefCounter::addWeakRef(refCount_);
            }
        }

        WeakRefPtr(const WeakRefPtr<T>& other)
        {
            set(other.ptr_,  other.refCount_);
        }

        template <class U >
        WeakRefPtr(const WeakRefPtr<U>& other)
        {
            set(static_cast<T*>(other.ptr_), other.refCount_);
        }

        WeakRefPtr(WeakRefPtr<T>&& other)
        {
            set(other.ptr_, other.refCount_);
            other.reset();
        }

        template <class U >
        WeakRefPtr(WeakRefPtr<U>&& other)
        {
            set(static_cast<T*>(other.ptr_), other.refCount_);
            other.reset();
        }

        WeakRefPtr(const RefPtr<T>& p)
        {
            if (p.get())
            {
                set(p.get(), RefCounter::refCountData(p.get()));
            }
        }

        ~WeakRefPtr()
        {
            reset();
        }

        WeakRefPtr operator = (T* ptr)
        {
            WeakRefPtr(ptr).swap(*this);
            return *this;
        }

        template<class U>
        WeakRefPtr operator = (U* ptr)
        {
            WeakRefPtr(ptr).swap(*this);
            return *this;
        }

        WeakRefPtr& operator = (const RefPtr<T>& p)
        {
            WeakRefPtr(p).swap(*this);
            return *this;
        }

        WeakRefPtr& operator = (const WeakRefPtr& other)
        {
            WeakRefPtr(other).swap(*this);
            return *this;
        }

        template<class U>
        WeakRefPtr& operator = (const WeakRefPtr<U>& other)
        {
            WeakRefPtr(other).swap(*this);
            return *this;
        }

        WeakRefPtr& operator = (WeakRefPtr&& other)
        {
            WeakRefPtr(std::move(other)).swap(*this);
            return *this;
        }

        template<class U>
        WeakRefPtr& operator = (WeakRefPtr<U>&& other)
        {
            WeakRefPtr(std::move(other)).swap(*this);
            return *this;
        }

        bool isNull() const
        {
            return !refCount_ || !refCount_->strongRefCount_;
        }

        bool isValid() const 
        {
            return refCount_ && refCount_->strongRefCount_ != 0;
        }

        RefPtr<T> lock() const
        {
            if (isValid())
            {
                return RefPtr<T>(ptr_);
            }
            else
                return nullptr;
        }

        void reset()
        {
            if (refCount_)
            {
                if (0 == RefCounter::releaseWeakRef(refCount_))
                {
                    RefCounter::destroyRefCountData(refCount_);
                }
            }

            refCount_ = nullptr;
            ptr_ = nullptr;
        }

    private:

        void swap(WeakRefPtr& other)
        {
            std::swap(this->ptr_, other.ptr_);
            std::swap(this->refCount_, other.refCount_);
        }
       
        void set(T* ptr,  RefCountData* data)
        {
            refCount_ = data;
            RefCounter::addWeakRef(refCount_);
            ptr_ = ptr;
        }
    };

}
