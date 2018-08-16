#pragma once

namespace Storm
{
    class WaitableEvent
    {
        STORM_NONCOPYABLE(WaitableEvent)

    public:

        enum State
        {
            Signal  = 0,
            Timeout = 1,
            Error = 2
        };

        WaitableEvent(bool manualReset = false, bool initialState = false)
            : handle_(nullptr)
        {
            handle_ = CreateEvent(nullptr, manualReset, initialState, nullptr);
        }

        ~WaitableEvent()
        {
            if (handle_)
            {
                CloseHandle(handle_);
            }
        }

        void set()
        {
            SetEvent(handle_);
        }

        void reset()
        {
            ResetEvent(handle_);
        }

        State wait(unsigned timeout = INFINITE)
        {
            DWORD ret = WaitForSingleObject(handle_, timeout);
            return ret == WAIT_OBJECT_0 ? Signal : WAIT_TIMEOUT ? Timeout : Error;
        }

        bool isSignal()
        {
            return (wait(0) == Signal);
        }

        HANDLE handle()
        {
            return handle_;
        }

    private:
        HANDLE handle_;
    };

    class Lockable
    {
        STORM_NONCOPYABLE(Lockable);

    public:
        Lockable() = default;
        virtual ~Lockable() = default;

        virtual void lock() = 0;
        virtual void unlock() = 0;
    };

    class Mutex : public Lockable
    {
        STORM_NONCOPYABLE(Mutex)
    public:
        Mutex()
            : handle_(nullptr)
        {
        }

        Mutex(bool initialOwner)
            : handle_(nullptr)
        {
            handle_ = CreateMutex(nullptr, initialOwner, nullptr);
        }

        bool create(bool initialOwner, const std::wstring& name)
        {
            handle_ = CreateMutex(nullptr, initialOwner, name.c_str());
            return handle_ != nullptr;
        }

        bool open(const std::wstring& name)
        {
            handle_ = OpenMutex(SYNCHRONIZE, FALSE, name.c_str());
            return handle_ != nullptr;
        }

        void close()
        {
            if (handle_)
            {
                CloseHandle(handle_);
                handle_ = nullptr;
            }
        }

        ~Mutex()
        {
            if (handle_)
            {
                CloseHandle(handle_);
            }
        }

        void lock() override
        {
            DWORD ret = WaitForSingleObject(handle_, INFINITE);
            (void)ret;
            if (ret == WAIT_ABANDONED)
            {
                close();
            }
        }

        void unlock() override
        {
            BOOL ret = ReleaseMutex(handle_);
            (void)ret;
            DAssert(ret);
        }

        HANDLE handle()
        {
            return handle_;
        }

    private:
        HANDLE handle_;
    };

    class CSLock : public Lockable
    {
        STORM_NONCOPYABLE(CSLock)

    public:
        CSLock()
        {
            ZeroMemory( &cslock_, sizeof(cslock_) );
            InitializeCriticalSection(&cslock_);
        }

        ~CSLock()
        {
            DeleteCriticalSection(&cslock_);
        }

        void lock() override
        {
            EnterCriticalSection(&cslock_);
        }

        void unlock() override
        {
            LeaveCriticalSection(&cslock_);
        }

        bool tryLock()
        {
            return TryEnterCriticalSection(&cslock_) ? true : false;
        }

    private:
        CRITICAL_SECTION cslock_;
    };

    class DummyLock : public Lockable
    {
        STORM_NONCOPYABLE(DummyLock)

    public:
        DummyLock()
        {
        }

        virtual ~DummyLock()
        {
        }

        void lock() override
        {
        }

        void unlock() override
        {
        }
    };

    class ScopeLovkV1
    {
        STORM_NONCOPYABLE(ScopeLovkV1)

    public:
        ScopeLovkV1(Lockable& lock)
            : lockable_(lock)
        {
            lockable_.lock();
        }

        ~ScopeLovkV1()
        {
            lockable_.unlock();
        }

    private:
        Lockable& lockable_;
    };
}
