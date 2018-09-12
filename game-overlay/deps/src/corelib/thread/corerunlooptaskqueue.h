#pragma once

namespace Storm
{
    namespace priv
    {
        class WrapTask
        {
            Q_LEAK_DETECTOR(WrapTask);
        public:
            WrapTask(const Callback0& func, const TimeTick& runTime)
                : func_(func), post(TimeTick::now()), run(runTime)
            {

            }

            WrapTask(Callback0&& func, const TimeTick& runTime)
                : func_(std::move(func)), post(TimeTick::now()), run(runTime)
            {

            }

            WrapTask(const Callback0& func)
                : func_(func), post(TimeTick::now())
            {

            }

            WrapTask(Callback0&& func)
                : func_(func), post(TimeTick::now())
            {

            }

            WrapTask(WrapTask&&) = default;

            WrapTask& operator=(WrapTask&&) = default;

            bool operator < (const WrapTask& other) const
            {
                if (run < other.run)
                {
                    return false;
                }
                if (run > other.run)
                {
                    return true;
                }
                return seq_ > other.seq_;
            }


            void invoke()
            {
                func_();
            }

            Callback0 func_;

            TimeTick post;

            TimeTick run;

            int seq_ = 0;
        };

        class RunloopTaskQueue
        {
            friend class CoreRunloop;

            std::mutex taskQueueLock_;
            std::deque<WrapTask> taskQueue_;
            CoreRunloop* runloop_ = nullptr;
            std::atomic<int> taskSeq_ = 0;

            explicit RunloopTaskQueue(CoreRunloop* runloop)
                : runloop_(runloop)
            {

            }

            void runloopDestroyed()
            {
                std::lock_guard<std::mutex> lock(taskQueueLock_);
                taskQueue_.clear();
                runloop_ = nullptr;
            }

            void add(WrapTask&& task);

        public:

            CoreRunloop* runloop()
            {
                std::lock_guard<std::mutex> lock(taskQueueLock_);
                return runloop_;
            }

            std::size_t size()
            {
                std::lock_guard<std::mutex> lock(taskQueueLock_);
                return taskQueue_.size();
            }

            void post(const Callback0& func)
            {
                add({ func });
            }

            void post(Callback0&& func)
            {
                add({ std::move(func) });
            }

            void postDelayed(const Callback0& func, int milliSeconds)
            {
                add({ func, TimeTick::now().addMilliSecs(milliSeconds) });
            }

            void postDelayed(Callback0&& func, int milliSeconds)
            {
                add({ std::move(func), TimeTick::now().addMilliSecs(milliSeconds) });
            }

            std::deque<WrapTask> lockSwapTaskQueue()
            {
                std::lock_guard<std::mutex> lock(taskQueueLock_);
                return std::move(taskQueue_);
            }
        };


    }
}
