#pragma once

namespace Ex
{
    class WaitableRef;

    class SchedulerWorker : public std::enable_shared_from_this<SchedulerWorker>
    {
    public:
        virtual void stop() = 0;

        virtual void schedule_waitable(WaitableRef& w) = 0;

        virtual void remove_waitable(WaitableRef& w) = 0;

        virtual void schedule_callback(Callback0&& func) = 0;

        virtual  std::thread::id thread_id() = 0;

    };

    class Scheduler
    {
        std::shared_ptr<SchedulerWorker> d_;
    public:
        Scheduler();
        ~Scheduler();

        void stop();

        static Scheduler& static_scheduler();

        std::shared_ptr<SchedulerWorker> worker();

        void schedule_waitable(WaitableRef& w);

        void schedule_signal(WaitableRef& w);

        void dispose_waitable(WaitableRef& w);

    };
}