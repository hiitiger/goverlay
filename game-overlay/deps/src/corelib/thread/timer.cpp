#include "stable.h"
#include "timer.h"
#include "corerunloop.h"

namespace Storm {

    class TimerCallback
    {
        friend class Timer;
        Timer* timer_;
        Q_LEAK_DETECTOR(TimerCallback);
    public:
        TimerCallback(Timer* timer) : timer_(timer)
        {

        }

        void run()
        {
            if (timer_)
            {
                timer_->timeOutEvent()(timer_);

                if (timer_ && timer_->repeat_)
                {
                    timer_->queueTimerTaskNext();
                }
            }
        }
    };


Timer::Timer()
{

}

Timer::~Timer()
{
    stop();
}

int Timer::interval() const 
{
    return  intervalMilliSeconds_;
}

void Timer::setInterval(int milliSeconds)
{
    if (intervalMilliSeconds_ != milliSeconds)
    {
        if (isActive())
        {
            stop();
            start(intervalMilliSeconds_);
        }
        else
            intervalMilliSeconds_ = milliSeconds;
    }
}

void Timer::setRepeat(bool r)
{
    DAssert(!isActive());
    repeat_ = r;
}


void Timer::startSingleShot()
{
    setRepeat(false);
    start();
}

void Timer::startSingleShot(int milliSeconds)
{
    setRepeat(false);
    start(milliSeconds);
}

void Timer::start()
{
    if (intervalMilliSeconds_ != -1)
    {
        start(intervalMilliSeconds_);
    }
}

void Timer::start(int milliSeconds)
{
    if (isActive())
    {
        if (intervalMilliSeconds_ = milliSeconds)
        {
            return;
        }
        else
            stop();
    }

    intervalMilliSeconds_ = milliSeconds;
    queueTimerTask();
}

void Timer::stop()
{
    if (isActive())
    {
        callback_->timer_ = nullptr;
        callback_ = nullptr;
    }
}

bool Timer::isActive() const
{
    return (bool)callback_;
}

Event<void(Timer*)>&  Timer::timeOutEvent()
{
    return timeOutEvent_;
}

void Timer::queueTimerTask()
{
    DAssert(!isActive());
    callback_ = std::make_shared<TimerCallback>((this));
    CoreRunloop::current()->postDelayed(Storm::bind(&TimerCallback::run, callback_), intervalMilliSeconds_);
}

void Timer::queueTimerTaskNext()
{
    DAssert(isActive());
    CoreRunloop::current()->postDelayed(Storm::bind(&TimerCallback::run, callback_), intervalMilliSeconds_);
}

}