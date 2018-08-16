#include "stable.h"
#include "schedule.h"
#include "thread/corerunloop.h"
#include "waitable.h"

namespace Ex
{
    class SchedulerImpl : public SchedulerWorker
    {
    public:

        std::atomic<bool> working_;
        std::mutex lock_;
        std::vector<WaitableRef> waitables_;
        std::unique_ptr<std::thread> thread_;
        Storm::CoreRunloop* runloop_ = nullptr;

    public:
        SchedulerImpl()
            : working_(false)
            
        {
            start();
        }

        ~SchedulerImpl()
        {
            stop();
        }

        void start()
        {
            std::lock_guard<std::mutex> lock(lock_);
            if (working_)
            {
                return;
            }
            thread_.reset(new std::thread([this]() { worker(); }));
            while (!working_);
        }

        void stop()
        {
            std::lock_guard<std::mutex> lock(lock_);
            if (working_)
            {
                working_ = false;
                thread_->join();
            }      
        }

        void worker()
        {
            Storm::CoreRunloop loop;

            runloop_ = &loop;
            working_ = true;

            while (working_)
            {
                if (working_)
                {
                    runloop_->runOnce();
                }
                if(working_)
                {
                    schedule();
                }
                if (working_)
                {
                    runloop_->tryIdleWait(10);
                }
            }

            runloop_ = nullptr;
        }

        void schedule()
        {
            for (auto& w : waitables_ )
            {
                w.check_signal();
            }
        }

        void schedule_waitable(WaitableRef& w)
        {
            std::lock_guard<std::mutex> lock(lock_);
            runloop_->post([this, w]() mutable {
                w.on_schedule();
                waitables_.push_back(w);
            });
        }

        void remove_waitable(WaitableRef& w)
        {
            std::lock_guard<std::mutex> lock(lock_);
            runloop_->post([this, w]() mutable {
                waitables_.erase(std::remove_if(waitables_.begin(), waitables_.end(), [w](auto ww) {return ww == w; }), waitables_.end());
            });
        }

        virtual void schedule_callback(Callback0&& func)
        {
            std::lock_guard<std::mutex> lock(lock_);
            runloop_->post(std::move(func));
        }

         std::thread::id thread_id()
        {
             return runloop_->threadId();
        }

    };

    Scheduler::Scheduler()
        : d_(new SchedulerImpl())
    {
    }

    Scheduler::~Scheduler()
    {
        stop();
    }

    void Scheduler::stop()
    {
        d_->stop();
    }

    Scheduler& Scheduler::static_scheduler()
    {
        static Scheduler s;
        return s;
    }

    std::shared_ptr<Ex::SchedulerWorker> Scheduler::worker()
    {
        return d_;
    }

    void Scheduler::schedule_waitable(WaitableRef& w)
    { 
        d_->schedule_waitable(w);
    }

    void Scheduler::dispose_waitable(WaitableRef& w)
    {
        d_->remove_waitable(w);
    }


}