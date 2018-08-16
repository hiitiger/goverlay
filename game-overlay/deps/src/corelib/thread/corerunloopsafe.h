#pragma once


namespace Storm
{

    class CoreRunloopSafe
    {
        friend class CoreRunloop;
        Q_LEAK_DETECTOR(CoreRunloopSafe);

        priv::RunloopTaskQueuePtr taskQueue_;

        explicit CoreRunloopSafe(const priv::RunloopTaskQueuePtr& taskQueue);

    public:

        CoreRunloop* runloopUnSafe() const;

        void post(const Callback0& func);
 
        void post(Callback0&& func);

        void postDelayed(const Callback0& func, int milliSeconds);

        void postDelayed(Callback0&& func, int milliSeconds);

    };

}