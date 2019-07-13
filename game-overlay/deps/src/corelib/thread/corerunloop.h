#pragma once

namespace Storm
{
    class CoreRunloop
    {
        STORM_NONCOPYABLE(CoreRunloop);

        static thread_local  CoreRunloop* threadRunloop_;
    protected:

        std::atomic<std::thread::id> createThreadId_;
        std::atomic<bool> running_ = true;
        std::atomic<bool> hasWork_ = false;
        HWND msgWindow_ = nullptr;

        CoreRunloopSafePtr runloopSafe_;
        priv::RunloopTaskQueuePtr taskQueue_;
        std::priority_queue<priv::WrapTask> delayQueue_;

        TimeTick nextDelay_;
        Event<void(CoreRunloop*)> sysQuitRecivedEvent_;
        std::optional<TimeTick> scheduledTimerTime_;
    public:
        static CoreRunloop* current();

        CoreRunloop();
        ~CoreRunloop();

        CoreRunloopSafePtr getSafePtr() const;

        Event<void(CoreRunloop*)>& sysQuitRecivedEvent();

        void post(const Callback0& func);
        void post(Callback0&& func);
 
        void postDelayed(const Callback0& func, int milliSeconds);
        void postDelayed(Callback0&& func, int milliSeconds);

        void quit();
        void postQuit();

        void run();
        virtual void runLoop();
        virtual void runOnce();
        virtual void tryIdleWait(unsigned int milliSeconds = std::numeric_limits<uint32_t>::max());
        virtual void idleWait(unsigned int milliSeconds);

        void scheduleWork();
        void schedule();

        std::thread::id threadId();

    protected:
        std::deque<priv::WrapTask> lockSwapTaskQueue();

        void runTaskQueue();

        void runDelayQueue();

        void scheduleDelayed(TimeTick runTime);
        void onSystemTimer();

        virtual void processSystemMessage();
 
    private:
        static LRESULT CALLBACK LoopWndProc(HWND window_handle,
            UINT message,
            WPARAM wparam,
            LPARAM lparam);
    };
}