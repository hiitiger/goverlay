#include "stable.h"
#include "waitable.h"
#include "thread/timer.h"

namespace Ex
{

    WaitableRef::WaitableRef(const std::shared_ptr<WaitableImpl>& d)
        : d_(d)
    {

    }

    Ex::WaitableState WaitableRef::state()
    {
        return d_->state();
    }

    Storm::TimeTick WaitableRef::timeout_time() const
    {
        return d_->timeoutTime_;
    }

    void WaitableRef::dispose()
    {
        d_->dispose();
        d_ = nullptr;
    }

    WaitableRef& WaitableRef::timeout(int milliSecs)
    {
        std::lock_guard<std::mutex> lock(d_->lock_);
        d_->timeout_ = std::chrono::milliseconds(milliSecs);
        return *this;
    }

    WaitableRef& WaitableRef::async_wait_on(Scheduler& s)
    {
        std::lock_guard<std::mutex> lock(d_->lock_);
        d_->schedule_on(s);
        return *this;
    }

    WaitableRef& WaitableRef::add(CallbackType&& cb)
    {
        std::lock_guard<std::mutex> lock(d_->lock_);
        d_->callbacks_.emplace_back(std::move(cb));
        return *this;
    }

    bool WaitableRef::operator==(const WaitableRef& other) const
    {
        return d_.get() == other.d_.get();
    }

    void WaitableRef::on_schedule()
    {
        d_->on_schedule();
    }

    void WaitableRef::check_signal()
    {
        d_->check_signal();
    }


    //////////////////////////////////////////////////////////////////////////

    class SysHandleWaitableImpl : public WaitableImpl
    {
        HANDLE handle_;
    public:
        SysHandleWaitableImpl(HANDLE h) : WaitableImpl(false), handle_(h)
        {

        }

        void nolock_dispose() override
        {

        }

        void nolock_check_signal() override
        {
            DWORD w = WaitForSingleObject(handle_, 0);
            if (w == WAIT_OBJECT_0)
            {
                nolock_signal();
            }
        }

        void nolock_on_schedule() override
        {

        }
    };


    class TimerWaitableImpl : public WaitableImpl
    {
        std::unique_ptr<Storm::Timer> timer_;
        int interval_ = 0;
    public:
        TimerWaitableImpl(int milliSeconds, bool once = false)
            : WaitableImpl(once)
            , interval_(milliSeconds)
        {
        }

        void nolock_dispose() override
        {
            timer_.reset(nullptr);
        }

        void nolock_check_signal() override
        {

        }

        void nolock_on_schedule() override
        {
            timer_.reset(new Storm::Timer());
            timer_->setRepeat(!is_once());
            timer_->setInterval(interval_);
            timer_->start();
            timer_->timeOutEvent().add([this](Storm::Timer*) {this->timer_trigger(); });
        }

    private:
        void timer_trigger()
        {
            __super::signal();
        }
    };

    WaitableRef waitable(HANDLE handle)
    {
        return WaitableRef(std::make_shared<SysHandleWaitableImpl>(handle));
    }

    WaitableRef waitable(int milliSeconds)
    {
        return WaitableRef(std::make_shared<TimerWaitableImpl>(milliSeconds));
    }

}