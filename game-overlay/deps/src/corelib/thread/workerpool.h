#pragma once

namespace Storm
{
    class WorkerPool
    {
        std::vector<std::thread> threads_;
        SyncQueue<std::function<void()>> tasks_;
        std::mutex lock_;
        std::atomic<bool> running_ = false;

        STORM_NONCOPYABLE(WorkerPool)
    public:
        explicit WorkerPool(unsigned int size = std::thread::hardware_concurrency());
        ~WorkerPool();

        static WorkerPool* pool();
        static void async(const Callback0& task);
        static void async(Callback0&& task);


        void add(const Callback0& task);
        void add(Callback0&& task);

        void restart(unsigned int size);
        void stop();

    protected:
        void start(unsigned int size);

        void threadRun();
    };
}
