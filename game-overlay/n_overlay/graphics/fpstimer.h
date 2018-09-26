#pragma once
#include <chrono>

class FpsTimer
{
public:
    FpsTimer();

    void reset();
    void start();
    float tick();

    std::uint32_t fps();

private:
    std::uint32_t frames_ = 0;
    std::uint32_t fps_ = 0;
    std::chrono::time_point<std::chrono::steady_clock> lastTime_;
};

inline FpsTimer::FpsTimer()
{
    start();
}

inline void FpsTimer::reset()
{
    fps_ = 0;
    frames_ = 0;
    lastTime_ = std::chrono::steady_clock::now();
}

inline void FpsTimer::start()
{
    lastTime_ = std::chrono::steady_clock::now();
}

float inline FpsTimer::tick()
{
    frames_ += 1;

    auto cur = std::chrono::steady_clock::now();
    std::chrono::duration<float, std::milli> diff = cur - lastTime_;
    auto delta = diff.count();

    if (delta >= 1000.0f)
    {
        fps_ = (int)((float)frames_ * 1000.0f / (float)(delta));
        lastTime_ = cur;
        frames_ = 0;
    }

    return delta;
}

inline std::uint32_t FpsTimer::fps()
{
    return fps_;
}
