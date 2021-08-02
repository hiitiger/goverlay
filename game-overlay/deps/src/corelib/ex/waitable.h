#pragma once
#include "schedule.h"

namespace Ex
{
    
enum class WaitableState
{
    Wait = 0,
    Signal,
    Timeout,
    Disposed
};

class WaitableImpl;

class WaitableRef
{
    friend class SchedulerWorker;
    std::shared_ptr<WaitableImpl> d_;
    typedef Storm::Callback<void(WaitableRef)>  CallbackType;
public:
    WaitableRef() = default;
    WaitableRef(const std::shared_ptr<WaitableImpl>& d);

    WaitableState state();
    Storm::TimeTick timeout_time() const;
    void dispose();

    WaitableRef& timeout(int milliSecs);
    WaitableRef& async_wait_on(Scheduler& s);
    WaitableRef& add(CallbackType&& cb);

    bool operator==(const WaitableRef& other) const;

    void on_schedule();
    void check_signal();

};

class WaitableImpl : public std::enable_shared_from_this<WaitableImpl>
{
    friend class WaitableRef;
public:
    WaitableImpl(bool once /*= false*/)
        : once_(once)
    {

    }

    WaitableState state() const
    {
        return state_;
    }

    bool is_once() const
    {
        return once_;
    }

    bool expired() const
    {
        if (state() == WaitableState::Timeout
            || state() == WaitableState::Disposed)
        {
            return true;
        }
        return false;
    }

    void reset()
    {
        if (state() == WaitableState::Timeout)
        {
            return;
        }
        if (!is_once())
        {
            state_ = WaitableState::Wait;
        }
    }

    void dispose()
    {
        std::lock_guard<std::mutex> lock(lock_);
        state_ = WaitableState::Disposed;
        s_.lock()->remove_waitable(WaitableRef(shared_from_this()));

        if (std::this_thread::get_id() != s_.lock()->thread_id())
        {
            s_.lock()->schedule_callback([share_this = shared_from_this()](){
                share_this->nolock_dispose();
            });
            return;
        }
    }

    void schedule_on(Scheduler& s)
    {
        s_ = s.worker();
        s_.lock()->schedule_waitable(WaitableRef(shared_from_this()));
    }

    void check_signal()
    {
        {
            std::lock_guard<std::mutex> lock(lock_);
            if (expired())
            {
                return;
            }
            nolock_check_signal();
        }
        
        if (state() != WaitableState::Signal && !timeoutTime_.isNull())
        {
            if (Storm::TimeTick::now() > timeoutTime_)
            {
                timeout();
            }
        }
    }

    void signal()
    {
        std::lock_guard<std::mutex> lock(lock_);
        if (expired())
        {
            return;
        }

        if (std::this_thread::get_id() != s_.lock()->thread_id())
        {
            s_.lock()->schedule_callback([share_this = shared_from_this()](){
                share_this->signal();
            });
            return;
        }

        nolock_signal();
    }

    void timeout()
    {
        std::lock_guard<std::mutex> lock(lock_);
        if (expired())
        {
            return;
        }

        if (std::this_thread::get_id() != s_.lock()->thread_id())
        {
            s_.lock()->schedule_callback([share_this = shared_from_this()](){
                share_this->timeout();
            });
            return;
        }

        nolock_timeout();
    }

    void nolock_signal()
    {
        state_ = WaitableState::Signal;
        nolock_invoke();
    }

    void nolock_timeout()
    {
        state_ = WaitableState::Timeout;
        nolock_invoke();
    }

    void nolock_invoke()
    {
        for (auto& cb :callbacks_)
        {
            cb(WaitableRef(shared_from_this()));
        }

        reset();
    }

    void on_schedule()
    {
        std::lock_guard<std::mutex> lock(lock_);
        nolock_on_schedule();
        scheduleTime_ = Storm::TimeTick::now();
        if (timeout_.count()>0)
        {
            timeoutTime_ = scheduleTime_.addMilliSecs(timeout_.count());
        }
    }


    virtual void nolock_dispose() = 0;
    virtual void nolock_check_signal() = 0;
    virtual void nolock_on_schedule() = 0;

protected:
    const bool once_;
    std::atomic<WaitableState> state_;

    Storm::TimeTick scheduleTime_;
    Storm::TimeTick timeoutTime_;
    std::chrono::milliseconds timeout_ = std::chrono::milliseconds::zero();
    std::weak_ptr<SchedulerWorker>  s_;
    std::mutex lock_;
    std::vector<Storm::Callback<void(WaitableRef)>> callbacks_;
};


template<class S, class T>
class EventWaitableImpl;

template<class R, class ...A, class T>
class EventWaitableImpl<R(A...), T> : public WaitableImpl, public Storm::Trackable<T>
{
public:
    EventWaitableImpl(Storm::Event<R(A...)>& event, bool once = false)
        : WaitableImpl(once)
    {
        event.add(&EventWaitableImpl::event_trigger, this);
    }

    void nolock_dispose() override
    {
        __super::removeAll();
    }

    void nolock_check_signal() override
    {

    }

    void nolock_on_schedule() override
    {

    }

private:
    void event_trigger(A... args)
    {
        (void)args;
        __super::signal();
    }
};

template<class S, class T>
inline WaitableRef waitable(Storm::Event<S, T>& event)
{
    return WaitableRef(std::make_shared<EventWaitableImpl<S, T>>(event));
}

WaitableRef waitable(HANDLE handle);

WaitableRef waitable(int milliSeconds);

}
