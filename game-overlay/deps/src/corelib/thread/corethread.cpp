#include "stable.h"
#include "corethread.h"
#include "dispatcher.h"
#include "corerunloopsafe.h"
#include "corerunloop.h"

namespace Storm {


class ThreadCenter
{
public:
    static CoreThread* fromUId(std::uint32_t);
    static void lockThreads();
    static void unlockThreads();
};

std::vector<CoreThread*> g_coreThreads;
std::mutex  g_coreThreadsLock;

CoreThread* ThreadCenter::fromUId(std::uint32_t id)
{
    std::lock_guard<std::mutex> lock(g_coreThreadsLock);
    return g_coreThreads[id];
}

void ThreadCenter::lockThreads()
{
    g_coreThreadsLock.lock();
}

void ThreadCenter::unlockThreads()
{
    g_coreThreadsLock.unlock();
}

CoreThread::CoreThread()
    : createThreadId_(std::this_thread::get_id())
{
    std::lock_guard<std::mutex> lock(g_coreThreadsLock);
    this->threadUId_ = g_coreThreads.size();
    g_coreThreads.push_back(this);
}

CoreThread::~CoreThread()
{
    stop();
    std::lock_guard<std::mutex> lock(g_coreThreadsLock);
    g_coreThreads[threadUId_] = nullptr;
}


void CoreThread::post(const Callback0& func)
{
    if (isRunning())
    {
        runloopSafe()->post(func);
    }
}

void CoreThread::post(Callback0&& func)
{
    if (isRunning())
    {
        runloopSafe()->post(std::move(func));
    }
}

void CoreThread::postDelayed(const Callback0& func, int milliSeconds)
{
    if (isRunning())
    {
        runloopSafe()->postDelayed(func, milliSeconds);
    }
}

void CoreThread::postDelayed(Callback0&& func, int milliSeconds)
{
    if (isRunning())
    {
        runloopSafe()->postDelayed(std::move(func), milliSeconds);
    }
}

void CoreThread::start()
{
    DAssert(!thread_);
    DAssert(createThreadId_ == std::this_thread::get_id());
    DAssert(!isRunning());
    std::lock_guard<std::mutex> lock(selfLock_);

    thread_.reset(new std::thread(&CoreThread::threadProc, this));
    while (!running_) {
        ;
    }
    started_ = true;
}

void CoreThread::stop()
{
    DAssert(createThreadId_ == std::this_thread::get_id());

    std::lock_guard<std::mutex> lock(selfLock_);

    if (started_ && !stopping_)
    {
        stopping_ = true;
        runloop_->post([=]() {
            threadDispatcher_->threadStopping();
            CoreRunloop::current()->postQuit();
        });
        thread_->join();
        thread_.reset(nullptr);
        stopping_ = false;
        started_ = false;
    }
}

bool CoreThread::isRunning()
{
    return running_;
}

std::uint32_t CoreThread::threadUId()
{
    return threadUId_;
}

std::thread::id CoreThread::threadId()
{
    return threadId_;
}

CoreRunloopSafePtr CoreThread::runloopSafe()
{
    return runloopSafe_;
}

ThreadDispatcherPtr CoreThread::threadDispatcher()
{
    return threadDispatcher_;
}

void CoreThread::threadProc()
{
    threadId_ = std::this_thread::get_id();

    CoreRunloop loop;
    runloop_ = &loop;
    runloopSafe_ = runloop_->getSafePtr();
    threadDispatcher_.reset(new ThreadDispatcher());
    threadDispatcher_->threadStart(runloopSafe_);

    init();

    running_ = true;

    runloop();

    running_ = false;

    uninit();

    threadDispatcher_->threadStopped();
    threadDispatcher_ = nullptr;
    runloop_ = nullptr;
    runloopSafe_ = nullptr;

    threadId_ = std::thread::id();
}


void CoreThread::init()
{

}

void CoreThread::runloop()
{
    runloop_->run();
}

void CoreThread::uninit()
{

}

}
