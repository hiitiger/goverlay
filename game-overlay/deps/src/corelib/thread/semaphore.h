#pragma once

namespace Storm {

    class Semaphore 
    {
        std::mutex mutex_;
        std::condition_variable cv_;
        int count_ = 0;

        STORM_NONCOPYABLE(Semaphore)

    public:
        Semaphore() = default;

        void release()
        {
            std::unique_lock<std::mutex> lock(mutex_);
            count_ += 1;
            cv_.notify_one();
        }

        void acquire()
        {
            std::unique_lock<std::mutex> lock(mutex_);
            while (count_ == 0)
            {
                cv_.wait(lock);
            }
            count_ -= 1;
        }
    };
}
