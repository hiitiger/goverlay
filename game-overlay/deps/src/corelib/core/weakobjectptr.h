#pragma once

namespace Storm
{
    class CoreObject;

    class WeakGuard
    {
        CoreObject* obj_;
        bool hasGuardPtr_;
        static std::multimap<CoreObject*, CoreObject**> ptrmap_;
        static std::mutex objmapLock_;

    public:
        WeakGuard(CoreObject* obj);

        void clearGuardObject(CoreObject* obj);

        void setsObj(CoreObject* obj);

        void setHasPtr(bool i);

        bool hasPtr() const;

        static void addObj(CoreObject** objaddr);

        static void removeObj(CoreObject** objaddr);

        static void clearObj(CoreObject* obj);

    };

    template<class T>
    class WeakObjectPtr
    {
        CoreObject* obj_;
    public:
        WeakObjectPtr()
            :obj_(nullptr)
        {
            ;
        }

        WeakObjectPtr(CoreObject* p)
            :obj_(p)
        {
            WeakGuard::addObj(&obj_);  //record address of obj_(point to object)
        }

        WeakObjectPtr(WeakObjectPtr const &p)
            : obj_(p.obj_)
        {
            WeakGuard::addObj(&obj_);
        }

        WeakObjectPtr(WeakObjectPtr&& p)
            : obj_(p.obj_)
        {
            WeakGuard::addObj(&obj_);
            WeakGuard::removeObj(&p.obj_);
            p.obj_ = nullptr;
        }

        WeakObjectPtr& operator=(WeakObjectPtr const & p)
        {
            if (obj_ != p.obj_)
            {
                WeakGuard::removeObj(&obj_);
                obj_ = p.obj_;
                WeakGuard::addObj(&obj_);
            }
            return *this;
        }

        WeakObjectPtr& operator=(WeakObjectPtr&& p)
        {
            if (obj_ != p.obj_)
            {
                WeakGuard::removeObj(&obj_);
                obj_ = p.obj_;
                WeakGuard::addObj(&obj_);
                WeakGuard::removeObj(&p.obj_);
                p.obj_ = nullptr;
            }

            return *this;
        }

        WeakObjectPtr& operator=(T* obj)
        {
            if (obj_ != obj)
            {
                WeakGuard::removeObj(&obj_);
                obj_ = obj;
                WeakGuard::addObj(&obj_);
            }

            return *this;
        }

        ~WeakObjectPtr()
        {
            WeakGuard::removeObj(&obj_);
        }

        bool isNull() const
        {
            return !obj_;
        }

        operator bool() const
        {
            return obj_ != nullptr;
        }

        operator T*() const
        {
            return static_cast<T*>(obj_);
        }

        T* operator->() const
        {
            return static_cast<T*>(obj_);
        }

        T& operator*() const
        {
            return *(static_cast<T*>(obj_));
        }

        T* data() const
        {
            return static_cast<T*>(obj_);
        }
    };

}