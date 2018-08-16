#pragma once

namespace Windows
{
    template <class T>
    class ComPtr
    {
    public:
        ComPtr()
            : p_(nullptr)
        {}

        ComPtr(T* obj)
            : p_(obj)
        {
            if (p_) p_->AddRef();
        }

        ComPtr(const ComPtr<T>& other)
            : p_(other.p_)
        {
            if (p_) p_->AddRef();
        }

        ~ComPtr()
        {
            release();
        }

        operator bool() const
        {
            return p_ != nullptr;
        }

        operator T*() const
        {
            return p_;
        }

        T& operator*() const
        {
            return *p_;
        }

        T* operator->() const
        {
            return p_;
        }

        T* get() const
        {
            return p_;
        }

        ComPtr& operator= (T* const newP)
        {
            if (newP != nullptr)
            {
                newP->AddRef();
            }
            release();
            p_ = newP;
            return *this;
        }

        ComPtr& operator= (const ComPtr<T>& newP)
        {
            return operator= (newP.p_);
        }

        T** resetAndGetPointerAddress()
        {
            release();
            p_ = nullptr;
            return &p_;
        }

        T** getPointerAdress()
        {
            return &p_;
        }

        HRESULT CoCreateInstance(REFCLSID classUUID, DWORD dwClsContext = CLSCTX_INPROC_SERVER)
        {
            HRESULT hr = ::CoCreateInstance(classUUID, 0, dwClsContext, __uuidof (T), (void**)resetAndGetPointerAddress());
            DAssert(hr != CO_E_NOTINITIALIZED);
            return hr;
        }

        template <class U>
        HRESULT QueryInterface(REFCLSID classUUID, ComPtr<U>& destObject) const
        {
            if (p_ == nullptr)
            {
                return E_POINTER;
            }
            return p_->QueryInterface(classUUID, (void**)destObject.resetAndGetPointerAddress());
        }

        template <class U>
        HRESULT QueryInterface(ComPtr<U>& destObject) const
        {
            return this->QueryInterface(__uuidof (U), destObject);
        }

    private:
        T* p_;

        void release()
        {
            if (p_ != nullptr)
            {
                p_->Release();
                p_ = nullptr;
            }
        }

        T** operator&();
    };


}

