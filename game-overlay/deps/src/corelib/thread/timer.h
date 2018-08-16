#pragma once

namespace Storm
{

    class Timer 
    {
        STORM_NONCOPYABLE(Timer);
        Q_LEAK_DETECTOR(Timer);
    public:
        Timer();

        ~Timer();

        void setInterval(int milliSeconds);
 
        void setRepeat(bool);

        void startSingleShot();

        void startSingleShot(int milliSeconds);

        void start();

        void start(int milliSeconds);

        void stop();

        bool isActive() const;

        Event<void(Timer*)>& timeOutEvent();

    protected:
        void queueTimerTask();

        void queueTimerTaskNext();

    private:
        friend class TimerCallback;

        int intervalMilliSeconds_ = -1;
        bool repeat_ = true;
        std::shared_ptr<TimerCallback> callback_;
        Event<void(Timer*)> timeOutEvent_;
    };

}