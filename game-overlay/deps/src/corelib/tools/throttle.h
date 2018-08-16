#pragma once
namespace Storm
{
    class Throttle
    {
        std::uint32_t rate_count_ = 0;
        std::chrono::milliseconds interval_ = std::chrono::milliseconds::zero();
        std::uint32_t frame_count_ = 0;
        std::chrono::milliseconds last_mark_tick_ = std::chrono::milliseconds::zero();
    public:
        explicit Throttle(std::uint32_t rate_count, std::uint32_t interval = 1000);

        void reset();
        bool tick();
    };


    inline Throttle::Throttle(std::uint32_t rate_count, std::uint32_t interval)
        : rate_count_(rate_count)
        , interval_(interval)
    {
        reset();
    }

    inline void Throttle::reset()
    {
        using namespace std::chrono;
        last_mark_tick_ = duration_cast<milliseconds>(steady_clock::now().time_since_epoch());
        frame_count_ = 0;
    }

    inline bool Throttle::tick()
    {
        using namespace std::chrono;
        frame_count_ += 1;
        milliseconds cur = duration_cast<milliseconds>(steady_clock::now().time_since_epoch());
        if ((cur - last_mark_tick_) >= interval_)
        {
            last_mark_tick_ = cur;
            frame_count_ = 1;
            return true;
        }

        if (frame_count_ > rate_count_)
        {
            return false;
        }
        else
        {
            return true;
        }
    }
}