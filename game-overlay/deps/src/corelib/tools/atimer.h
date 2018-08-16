#pragma once

namespace stdxx 
{
    class timer
    {
        struct timer_thread
        {
            std::condition_variable cv_;
            std::mutex lock_;
            std::multimap<std::uint64_t, std::function<void()>> timer_tasks_;
            std::atomic<bool> quit_ = false;

            std::thread thread_;
            
            timer_thread()
                : thread_([this] {run(); })
            {
            }
            ~timer_thread()
            {
                quit_ = true;
                cv_.notify_one();
                thread_.join();
            }

            void add(std::uint32_t ms, std::function<void()>&& t)
            {
                std::unique_lock<std::mutex> lock(lock_);
                using namespace std::chrono;
                auto now = duration_cast<microseconds>(steady_clock::now().time_since_epoch());
                now += milliseconds(ms);
                timer_tasks_.insert(std::make_pair(now.count(), std::move(t)));
                cv_.notify_one();
            }

            void run()
            {
                using namespace std::chrono;
                while (!quit_)
                {
                    std::vector<std::function<void()>> timer_tasks;
                    {
                        std::unique_lock<std::mutex> lock(lock_);

                        while (timer_tasks_.empty() && !quit_)
                        {
                            cv_.wait_for(lock, std::chrono::milliseconds(1));
                        }

                        auto now = duration_cast<microseconds>(steady_clock::now().time_since_epoch());
                        auto it = timer_tasks_.upper_bound(now.count());
                        std::transform(std::make_move_iterator(timer_tasks_.begin()), std::make_move_iterator(it), std::back_inserter(timer_tasks),
                                        [](auto && pair) { return std::move(pair.second);});

                        timer_tasks_.erase(timer_tasks_.begin(), it);
                    }
                    for (auto task : timer_tasks)
                    {
                        task();
                    }
                }
            }
        };

        static timer_thread&  timer_thread_()
        {
            static timer_thread th;
            return th;
        }

        struct timer_cb
        {
            std::uint32_t interval_;
            std::function<bool()> cb_;

            void run()
            {
                if (cb_())
                {
                    timer_thread_().add(interval_, [this] {run(); });
                }
                else
                {
                    delete this;
                }
            }
        };

    public:
        static void forever(std::uint32_t milliseconds, std::function<bool()>&& callback);
        static void once(std::uint32_t milliseconds, std::function<void()>&& callback);
    };

    inline void timer::forever(std::uint32_t milliseconds, std::function<bool()>&& callback)
    {
        auto cb = new timer_cb{ milliseconds, std::move(callback) };
        timer_thread_().add(milliseconds, [cb] { cb->run(); });
    }

    inline void timer::once(std::uint32_t milliseconds, std::function<void()>&& callback)
    {
        timer_thread_().add(milliseconds, std::move(callback));
    }

}