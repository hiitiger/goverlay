#pragma once

namespace Storm {


class TimeTick
{
    std::chrono::microseconds microSecs_ = std::chrono::microseconds::zero();
private:
    template<class Clock >
    TimeTick(std::chrono::time_point<Clock> tp)
        : microSecs_(std::chrono::duration_cast<std::chrono::microseconds>(tp.time_since_epoch()))
    {
    }
public:
    TimeTick() = default;

    static TimeTick highResNow()
    {
        using namespace std::chrono;
        return high_resolution_clock::now();
    }

    static TimeTick now()
    {
        using namespace std::chrono;
        return steady_clock::now();
    }

    bool isNull() const
    {
        return microSecs_.count() == 0;
    }

    void setZero()
    {
        microSecs_ = std::chrono::microseconds::zero();
    }

    int64_t microSecs() const
    {
        return microSecs_.count();
    }

    int64_t milliSecs() const
    {
        using namespace std::chrono;
        return duration_cast<milliseconds>(microSecs_).count();
    }

    int64_t secs() const
    {
        using namespace std::chrono;
        return duration_cast<seconds>(microSecs_).count();
    }

    TimeTick& addMicroSecs(int64_t microSeconds)
    {
        using namespace std::chrono;
        microSecs_ += microseconds(microSeconds);
        return *this;
    }

    TimeTick& minusMicroSecs(int64_t microSeconds)
    {
        using namespace std::chrono;
        microSecs_ -= microseconds(microSeconds);
        return *this;
    }

    TimeTick& addMilliSecs(int64_t milliSconds)
    {
        using namespace std::chrono;
        microSecs_ += milliseconds(milliSconds);
        return *this;
    }

    TimeTick& minusMilliSecs(int64_t milliSconds)
    {
        using namespace std::chrono;
        microSecs_ -= milliseconds(milliSconds);
        return *this;
    }

    template<class _R, class _P>
    TimeTick& operator += (std::chrono::duration<_R, _P> t)
    {
        microSecs_ += t;
        return *this;
    }

    template<class _R, class _P>
    TimeTick& operator -= (std::chrono::duration<_R, _P> t)
    {
        microSecs_ -= t;
        return *this;
    }

    TimeTick& operator += (const TimeTick& t)
    {
        microSecs_ += t.microSecs_;
        return *this;
    }

    TimeTick& operator -= (const TimeTick& t)
    {
        microSecs_ -= t.microSecs_;
        return *this;
    }

    TimeTick operator - (const TimeTick& t)
    {
        TimeTick result = *this;
        result -= t;
        return result;
    }

    TimeTick operator + (const TimeTick& t)
    {
        TimeTick result = *this;
        result += t;
        return result;
    }

    bool operator==(const TimeTick& t)const { return microSecs_ == t.microSecs_; }
    bool operator!=(const TimeTick& t)const { return microSecs_ != t.microSecs_; }
    bool operator <(const TimeTick& t)const { return microSecs_  < t.microSecs_; }
    bool operator<=(const TimeTick& t)const { return microSecs_ <= t.microSecs_; }
    bool operator >(const TimeTick& t)const { return microSecs_  > t.microSecs_; }
    bool operator>=(const TimeTick& t)const { return microSecs_ >= t.microSecs_; }
};

}
