#include "stable.h"
#include "corerunloopsafe.h"
#include "corerunlooptaskqueue.h"
#include "corerunloop.h"

namespace Storm {


CoreRunloopSafe::CoreRunloopSafe(const priv::RunloopTaskQueuePtr& taskQueue)
    : taskQueue_(taskQueue)
{

}

CoreRunloop* CoreRunloopSafe::runloopUnSafe() const
{
    return taskQueue_->runloop();
}

void CoreRunloopSafe::post(const Callback0& func)
{
    taskQueue_->post(func);
}


void CoreRunloopSafe::post(Callback0&& func)
{
    taskQueue_->post(std::move(func));
}

void CoreRunloopSafe::postDelayed(const Callback0& func, int milliSeconds)
{
    taskQueue_->postDelayed(func, milliSeconds);
}

void CoreRunloopSafe::postDelayed(Callback0&& func, int milliSeconds)
{
    taskQueue_->postDelayed(std::move(func), milliSeconds);
}

}

