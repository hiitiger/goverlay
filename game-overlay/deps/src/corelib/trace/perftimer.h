#pragma once

namespace Windows {


    class PerfCounter
    {
        int64_t tickPerSec_ = 0;
        double tickPerMicroSec_ = 0;

    public:
        PerfCounter()
        {
            QueryPerformanceFrequency((LARGE_INTEGER*)&tickPerSec_);
            tickPerMicroSec_ = tickPerSec_ / 1000000.0;
        }

        int64_t systemTickFreqency() const
        {
            return tickPerSec_;
        }

        int64_t systemTick() const
        {
            int64_t now;
            QueryPerformanceCounter((LARGE_INTEGER*)&now);
            return now;
        }

        double tickPerMicroSec() const
        {
            return tickPerMicroSec_;
        }

        int64_t microSecsNow() const
        {
            int64_t now;
            QueryPerformanceCounter((LARGE_INTEGER*)&now);
            return static_cast<int64_t>(now / tickPerMicroSec_);
        }

        int64_t milliSecsNow() const
        {
            int64_t now;
            QueryPerformanceCounter((LARGE_INTEGER*)&now);
            return static_cast<int64_t>(now / (1000.0*tickPerMicroSec_));
        }
    };

}



