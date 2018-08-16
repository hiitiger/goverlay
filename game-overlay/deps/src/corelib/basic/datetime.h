#pragma once

namespace Storm
{
    enum class TimeSpec
    {
        UTC,
        Local
    };

    class DateTime
    {
        int year_= 0;
        int month_= 0;
        int dayOfWeek_= 0;
        int dayOfMonth_= 0;
        int dayOfYear_= 0;
        int hour_= 0;
        int minute_= 0;
        int sec_= 0;
        int milliSec_= 0;
        int64_t milliSecsUTC_ = 0;
        TimeSpec timeSpec_ = TimeSpec::Local;
    public:
        DateTime() = default;

        static DateTime now();
        static DateTime nowUTC();
        static int64_t currentMilliSecsSinceEpoch();

        static DateTime fromUTCTimeStamp(std::uint32_t sec);
        static DateTime fromTimeStamp(std::uint32_t sec);

        void addMilliSecs(int64_t msecs);

        TimeSpec timeSpec() const { return timeSpec_; }
        int year() const { return year_; }
        int month() const { return month_; }
        int dayOfWeek() const { return dayOfWeek_; }
        int dayOfMonth() const { return dayOfMonth_; }
        int dayOfYear() const { return dayOfYear_; }
        int hour() const { return hour_; }
        int minute() const { return minute_; }
        int sec() const { return sec_; }
        int milliSec() const { return milliSec_; }

    };

    inline DateTime DateTime::now()
    {
        using namespace std::chrono;
        auto timePoint = system_clock::now();
        auto cur = timePoint.time_since_epoch();
        milliseconds ms = duration_cast<milliseconds>(cur);
        int milliSecs = static_cast<int>(ms.count() % 1000);
        time_t curtime = system_clock::to_time_t(timePoint);
        
        //std::tm* curtm = localtime(&curtime);
        std::tm buf;
        localtime_s(&buf, &curtime);
        std::tm* curtm = &buf;

        DateTime dateTime;
        dateTime.year_ = curtm->tm_year + 1900;
        dateTime.month_ = curtm->tm_mon + 1;
        dateTime.dayOfWeek_ = curtm->tm_wday + 1;
        dateTime.dayOfMonth_ = curtm->tm_mday;
        dateTime.dayOfYear_ = curtm->tm_yday;
        dateTime.hour_ = curtm->tm_hour;
        dateTime.minute_ = curtm->tm_min;
        dateTime.sec_ = curtm->tm_sec;
        dateTime.milliSec_ = milliSecs;
        dateTime.milliSecsUTC_ = ms.count();
        dateTime.timeSpec_ = TimeSpec::Local;
        return dateTime;
    }

    inline DateTime DateTime::nowUTC()
    {
        using namespace std::chrono;
        auto timePoint = system_clock::now();
        auto cur = timePoint.time_since_epoch();
        milliseconds ms = duration_cast<milliseconds>(cur);
        int milliSecs = static_cast<int>(ms.count() % 1000);
        time_t curtime = system_clock::to_time_t(timePoint);
        //std::tm* curtm = gmtime(&curtime);
        std::tm buf;
        gmtime_s(&buf, &curtime);
        std::tm* curtm = &buf;

        DateTime dateTime;
        dateTime.year_ = curtm->tm_year + 1900;
        dateTime.month_ = curtm->tm_mon + 1;
        dateTime.dayOfWeek_ = curtm->tm_wday + 1;
        dateTime.dayOfMonth_ = curtm->tm_mday;
        dateTime.dayOfYear_ = curtm->tm_yday;
        dateTime.hour_ = curtm->tm_hour;
        dateTime.minute_ = curtm->tm_min;
        dateTime.sec_ = curtm->tm_sec;
        dateTime.milliSec_ = milliSecs;
        dateTime.milliSecsUTC_ = ms.count();
        dateTime.timeSpec_ = TimeSpec::UTC;
        return dateTime;
    }

    inline int64_t DateTime::currentMilliSecsSinceEpoch()
    {
        using namespace std::chrono;
        milliseconds ms = duration_cast<milliseconds>(
            system_clock::now().time_since_epoch()
            );
        return ms.count();
    }

    inline DateTime DateTime::fromUTCTimeStamp(std::uint32_t sec)
    {
        DateTime dateTime;
        time_t curtime = sec;
        std::tm buf;
        gmtime_s(&buf, &curtime);
        std::tm* curtm = &buf;

        dateTime.year_ = curtm->tm_year + 1900;
        dateTime.month_ = curtm->tm_mon + 1;
        dateTime.dayOfWeek_ = curtm->tm_wday + 1;
        dateTime.dayOfMonth_ = curtm->tm_mday;
        dateTime.dayOfYear_ = curtm->tm_yday;
        dateTime.hour_ = curtm->tm_hour;
        dateTime.minute_ = curtm->tm_min;
        dateTime.sec_ = curtm->tm_sec;
        dateTime.milliSec_ = 0;
        dateTime.milliSecsUTC_ = (int64_t)sec * 1000;
        dateTime.timeSpec_ = TimeSpec::UTC;
        return dateTime;
    }

    inline DateTime DateTime::fromTimeStamp(std::uint32_t sec)
    {
        DateTime dateTime;
        time_t curtime = sec ;
        std::tm buf;
        localtime_s(&buf, &curtime);
        std::tm* curtm = &buf;

        dateTime.year_ = curtm->tm_year + 1900;
        dateTime.month_ = curtm->tm_mon + 1;
        dateTime.dayOfWeek_ = curtm->tm_wday + 1;
        dateTime.dayOfMonth_ = curtm->tm_mday;
        dateTime.dayOfYear_ = curtm->tm_yday;
        dateTime.hour_ = curtm->tm_hour;
        dateTime.minute_ = curtm->tm_min;
        dateTime.sec_ = curtm->tm_sec;
        dateTime.milliSec_ = 0;
        dateTime.milliSecsUTC_ = (int64_t)sec * 1000;
        dateTime.timeSpec_ = TimeSpec::Local;
        return dateTime;
    }

    inline void DateTime::addMilliSecs(int64_t msecs)
    {
        int64_t utcTimeMSecs = milliSecsUTC_ + msecs;
        int milliSecs = static_cast<int>(utcTimeMSecs % 1000);
        time_t curtime = utcTimeMSecs / 1000;

        std::tm buf;
        if (timeSpec_ == TimeSpec::Local)
        {
            localtime_s(&buf, &curtime);
        }
        else
        {
            gmtime_s(&buf, &curtime);
        }
        std::tm* curtm = &buf;

        year_ = curtm->tm_year + 1900;
        month_ = curtm->tm_mon + 1;
        dayOfWeek_ = curtm->tm_wday + 1;
        dayOfMonth_ = curtm->tm_mday;
        dayOfYear_ = curtm->tm_yday;
        hour_ = curtm->tm_hour;
        minute_ = curtm->tm_min;
        sec_ = curtm->tm_sec;
        milliSec_ = milliSecs;
        milliSecsUTC_ = utcTimeMSecs;
    }

    inline std::ostream& operator<<(std::ostream& os, const DateTime& dateTime)
    {
        char output[32] = { 0 };
        sprintf_s(output, "%04d-%02d-%02d %02d:%02d:%02d:%03d", dateTime.year(), dateTime.month(), dateTime.dayOfMonth(), dateTime.hour(), dateTime.minute(), dateTime.sec(), dateTime.milliSec());
        os << output;
        return os;
    }

    inline std::string toString(const DateTime& dateTime)
    {
        std::stringstream ss;
        ss << dateTime;
        return ss.str();
    }

}