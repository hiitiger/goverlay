#pragma once

namespace Storm 
{
    class RWLock
    {
        std::atomic<bool> writeLock_ = false;
        std::atomic<int> readerCount_ = 0;
        STORM_NONCOPYABLE(RWLock)
    public:
        RWLock() = default;

        void lockWrite();
        void unlockWrite();

        void lockRead();
        void unlockRead();
    };

    inline void RWLock::lockWrite()
    {
        bool expected = false;
        while (!writeLock_.compare_exchange_strong(expected, true))
        {
            expected = false;
        }

        while (readerCount_)
        {
        }
    }

    inline void RWLock::unlockWrite()
    {
        writeLock_.store(false);
    }

    inline void RWLock::lockRead()
    {
        while (writeLock_)
        {
        }
        readerCount_ += 1;
    }

    inline void RWLock::unlockRead()
    {
        readerCount_ -= 1;
    }

    class ScopedReadLock
    {
        RWLock& lock_ ;
    public:
        ScopedReadLock(RWLock& lock) :lock_(lock)
        {
            lock_.lockRead();
        }

        ~ScopedReadLock()
        {
            lock_.unlockRead();
        }
    };

    class ScopedWriteLock
    {
        RWLock& lock_;
    public:
        ScopedWriteLock(RWLock& lock) :lock_(lock)
        {
            lock_.lockWrite();
        }

        ~ScopedWriteLock()
        {
            lock_.unlockWrite();
        }
    };
}
