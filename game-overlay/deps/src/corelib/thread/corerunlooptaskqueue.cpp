#include "stable.h"
#include "corerunlooptaskqueue.h"
#include "corerunloop.h"

namespace Storm
{
    

void priv::RunloopTaskQueue::add(WrapTask&& task)
{
    std::lock_guard<std::mutex> lock(taskQueueLock_);
    if (runloop_)
    {
        taskSeq_ += 1;
        task.seq_ = taskSeq_;

        taskQueue_.push_back(std::move(task));
        runloop_->scheduleWork();
    }
}

}