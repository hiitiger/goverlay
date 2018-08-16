#pragma once

namespace Storm
{
    class CoreThread 
    {
        STORM_NONCOPYABLE(CoreThread);

    protected:
        std::mutex  selfLock_;
        std::atomic<bool> running_ = false;
        std::atomic<bool> stopping_ = false;

        std::atomic<bool> started_ = false;
        std::uint32_t threadUId_ = 0;
        std::thread::id createThreadId_;
        std::atomic<std::thread::id> threadId_;
        std::unique_ptr<std::thread> thread_;
        CoreRunloop* runloop_ = nullptr;
        CoreRunloopSafePtr runloopSafe_;
        ThreadDispatcherPtr threadDispatcher_ = nullptr;

    public:
        CoreThread();
        virtual ~CoreThread();

        void post(const Callback0& func);
        void post(Callback0&& func);

        void postDelayed(const Callback0& func, int milliSeconds);
        void postDelayed(Callback0&& func, int milliSeconds);

        void start();
        void stop();

        bool isRunning();

        std::uint32_t threadUId();
        std::thread::id threadId();

        CoreRunloopSafePtr runloopSafe();
        ThreadDispatcherPtr threadDispatcher();

    protected:
        virtual void threadProc();
        virtual void init();
        virtual void runloop();
        virtual void uninit();
    };

}