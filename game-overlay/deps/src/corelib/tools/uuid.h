#pragma once

namespace Storm
{
    namespace priv
    {
        class SnowflakeMultiThread
        {
            std::atomic<bool> lock_ = false;
        public:
            void lock()
            {
                bool expected = false;
                while (!lock_.compare_exchange_strong(expected, true))
                {
                    expected = false;
                }

            }
            void unlock()
            {
                lock_.store(false);
            }
        };

        class SnowflakeSingleThread
        {
        public:
            void lock() { ; }
            void unlock() { ; }
        };

        template<bool>
        struct SnowflakeThreadPolicy;

        template<>
        struct SnowflakeThreadPolicy<true>
        {
            using Lock = SnowflakeMultiThread;
        };

        template<>
        struct SnowflakeThreadPolicy<false>
        {
            using Lock = SnowflakeSingleThread;
        };

        template<bool T>
        using SnowflakeThreadLock = typename SnowflakeThreadPolicy<T>::Lock;

        inline void usleep(std::int64_t microsecs_to_sleep)
        {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
            LARGE_INTEGER ft;
            ft.QuadPart = -(10 * microsecs_to_sleep);
            HANDLE timer = CreateWaitableTimer(nullptr, TRUE, nullptr);
            SetWaitableTimer(timer, &ft, 0, nullptr, nullptr, 0);
            WaitForSingleObject(timer, INFINITE);
            CloseHandle(timer);
#else
            std::this_thread::sleep_for(std::chrono::microseconds(microsecs_to_sleep));
#endif
        }
    }


    template<bool t = true>
    class Snowflake
    {
    public:
        Snowflake() = default;

        void setEpoch(std::int64_t epoch);
        void setMachine(std::uint32_t machine);

        int64_t generate();

    private:
        std::chrono::milliseconds epoch_ = std::chrono::milliseconds::zero();
        std::chrono::milliseconds last_ms_ = std::chrono::milliseconds::zero();
        std::uint32_t machine_ = 0;
        std::uint32_t sequence_ = 0;

        std::atomic<int64_t> exception_counter_ = 0;
        priv::SnowflakeThreadLock<t> lock_;
    };

    
    template<bool t> inline void Snowflake<t>::setEpoch(std::int64_t epoch)
    {
        epoch_ = std::chrono::milliseconds(epoch);
    }

    template<bool t> inline void Snowflake<t>::setMachine(std::uint32_t machine)
    {
        machine_ = machine;
    }

    template<bool t> inline int64_t Snowflake<t>::generate()
    {
        std::lock_guard<priv::SnowflakeThreadLock<t>> lock(lock_);

        int64_t value = 0;
        using namespace std::chrono;
        microseconds microsecs = duration_cast<microseconds>(system_clock::now().time_since_epoch());
        milliseconds cur = duration_cast<milliseconds>(microsecs) ;
        milliseconds ms = cur - epoch_;

        while (ms < last_ms_)
        {
            exception_counter_ += 1;
            microsecs = duration_cast<microseconds>(system_clock::now().time_since_epoch());
            cur = duration_cast<milliseconds>(microsecs);
            ms = cur - epoch_;
        }

        value |= ms.count() << 22;

        value |= (machine_ & 0x3ff) << 12;

        if (ms == last_ms_)
        {
            value |= (sequence_++) & 0xfff;
            if (sequence_ == 0x1000)
            {
                sequence_ = 0;
            }
            if (sequence_ == 0)
            {
                microseconds sleep_to = duration_cast<microseconds>(cur + milliseconds(1));
                while (sleep_to >= duration_cast<microseconds>(system_clock::now().time_since_epoch()))
                {
                    microseconds microsecs_to_sleep = (sleep_to - microsecs);
                    priv::usleep(microsecs_to_sleep.count());
                }
            }
        }
        else
        {
            sequence_ = 0;
        }

        last_ms_ = ms;

        return value;
    }

}