
#pragma once

#include <string>
#include <memory>
#include <mutex>
#include <atomic>
#include <functional>
#include <iostream>
#include <queue>

#include <uv.h>

using task_type = std::function<void(void)>;

template <typename Elem, typename T2 = int, typename Lck = std::mutex>
class async_queue
{
    async_queue(const async_queue &) = delete;
    async_queue &operator=(const async_queue &) = delete;

  public:
    using callback_type = std::function<void(task_type &)>;
    async_queue(uv_loop_t *loop, callback_type &&cb)
        : h_((uv_async_t *)malloc(sizeof(uv_async_t))), closed_(false), cb_(std::move(cb)), capacity_(0)
    {
        ::uv_async_init(loop, h_, async_callback);
        h_->data = this;
    }

    ~async_queue()
    {
        uv_close((uv_handle_t *)h_, [](uv_handle_t *handle) {
            free(handle);
        });
    }
    int async_call(Elem &&e, uint64_t ts = 0)
    {
        if (closed_)
        {
            return -1;
        }

        {
            std::lock_guard<Lck> guard(lock_);
            if (capacity_ && q_.size() > capacity_)
            {
                q_.pop();
            }
            q_.push(std::move(e));
        }

        return !uv_async_send(h_) ? 0 : -1;
    }
    size_t size() const
    {
        std::lock_guard<Lck> guard(lock_);
        return q_.size();
    }
    bool empty() const
    {
        return size() == 0;
    }
    void close()
    {
        if (!empty())
        {
            std::cerr << "@trace n async queue is not empty" << std::endl;
        }
        closed_ = true;
    }
    bool closed() const
    {
        return closed_;
    }
    void set_priority(int prio) {}
    void set_capacity(size_t capacity)
    {
        capacity_ = capacity;
    }
    void clear()
    {
        std::lock_guard<Lck> guard(lock_);
        std::queue<Elem> empty;
        std::swap(q_, empty);
    }
    uint64_t last_pop_ts() const
    {
        return 0;
    }

  private:
    static void async_callback(uv_async_t *handle)
    {
        reinterpret_cast<async_queue *>(handle->data)->on_event();
    }
    void on_event()
    {
        std::unique_lock<Lck> lock(lock_);
        while (!q_.empty())
        {
            Elem e(std::move(q_.front()));
            q_.pop();
            lock.unlock();
            cb_(e);
            lock.lock();
        }
    }

  private:
    uv_async_t *h_ = nullptr;
    std::atomic<bool> closed_;
    mutable Lck lock_;
    std::queue<Elem> q_;
    callback_type cb_;
    size_t capacity_;
};

class node_async_call
{
  public:
    static void async_call(task_type &&cb)
    {
        node_async_call::instance().node_queue_->async_call(std::move(cb));
    }

  private:
    using node_queue_type = async_queue<task_type>;
    node_async_call();
    ~node_async_call();
    static node_async_call &instance() { return s_instance_; }
    void run_task(task_type &task)
    {
        task();
    }
    std::unique_ptr<node_queue_type> node_queue_;
    static node_async_call s_instance_;
};
