#include "stable.h"
#include "workerpool.h"

namespace Storm {


WorkerPool::WorkerPool(unsigned int size /*= std::thread::hardware_concurrency()*/)
{
    start(size);
}

WorkerPool::~WorkerPool()
{
    stop();
}

WorkerPool* WorkerPool::pool()
{
    static WorkerPool pool;
    return &pool;
}

void WorkerPool::async(const Callback0& task)
{
    pool()->add(task);
}

void WorkerPool::async(Callback0&& task)
{
    pool()->add(std::move(task));
}

void WorkerPool::add(const Callback0& task)
{
    tasks_.enqueue(task);
}

void WorkerPool::add(Callback0&& task)
{
    tasks_.enqueue(std::move(task));
}

void WorkerPool::restart(unsigned int size)
{
    stop();
    start(size);
}

void WorkerPool::stop()
{
    std::lock_guard<std::mutex> lock(lock_);
    if (!running_)
    {
        return;
    }

    running_ = false;
    tasks_.stop();
    for (auto& thread : threads_)
    {
        thread.join();
    }
    threads_.clear();
}

void WorkerPool::start(unsigned int size)
{
    std::lock_guard<std::mutex> lock(lock_);
    if (running_)
    {
        return;
    }
    running_ = true;
    tasks_.start();
    for (unsigned int i = 0; i != size; ++i)
    {
        threads_.push_back(std::thread(&WorkerPool::threadRun, this));
    }
}

void WorkerPool::threadRun()
{
    while (true)
    {
        std::function<void()> func;
        if (tasks_.dequeue(func))
        {
            //if get task already just run it 
            func();
        }

        if (!running_)
        {
            break;
        }
    }
}
}
