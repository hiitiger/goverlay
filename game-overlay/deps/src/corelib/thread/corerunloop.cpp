#include "stable.h"
#include "corerunlooptaskqueue.h"
#include "corerunloopsafe.h"
#include "corerunloop.h"

namespace Storm
{
    static const int k_MsgLoopWorkMsg = WM_APP + 1;
    static const int k_MsgLoopWakeupMsg = WM_APP + 2;

    LRESULT CALLBACK CoreRunloop::LoopWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        CoreRunloop *pThis = (CoreRunloop*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
        DAssert(pThis); pThis;
        if (uMsg == k_MsgLoopWorkMsg)
        {
            pThis->runTaskQueue();
        }
        else if (uMsg == WM_TIMER)
        {
            pThis->onSystemTimer();
        }
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }


thread_local CoreRunloop* CoreRunloop::threadRunloop_ = nullptr;

CoreRunloop* CoreRunloop::current()
{
    return threadRunloop_;
}

CoreRunloop::CoreRunloop()
    : createThreadId_(std::this_thread::get_id())
{
    DAssert(threadRunloop_ == nullptr);
    threadRunloop_ = this;
    taskQueue_.reset(new priv::RunloopTaskQueue(this));
    runloopSafe_.reset(new CoreRunloopSafe(taskQueue_));

    msgWindow_ = ::CreateWindowW(L"STATIC", L"XXXMsgLoopWindow", WS_POPUP, 0, 0, 0, 0, HWND_MESSAGE, nullptr, nullptr, 0);
    ::SetWindowLongPtr(msgWindow_, GWLP_USERDATA, (LONG_PTR)this);
    ::SetWindowLongPtr(msgWindow_, GWLP_WNDPROC, (LONG_PTR)LoopWndProc);
}

CoreRunloop::~CoreRunloop()
{
    taskQueue_->runloopDestroyed();

    ::DestroyWindow(msgWindow_);
    msgWindow_ = nullptr;
    threadRunloop_ = nullptr;
}

CoreRunloopSafePtr CoreRunloop::getSafePtr() const
{
    return runloopSafe_;
}

Event<void(CoreRunloop*)>& CoreRunloop::sysQuitRecivedEvent()
{
    return sysQuitRecivedEvent_;
}

void CoreRunloop::post(const Callback0& func)
{
    taskQueue_->post(func);
}

void CoreRunloop::post(Callback0&& func)
{
    taskQueue_->post(std::move(func));
}

void CoreRunloop::postDelayed(const Callback0& func, int milliSeconds)
{
    taskQueue_->postDelayed(func, milliSeconds);
}

void CoreRunloop::postDelayed(Callback0&& func, int milliSeconds)
{
    taskQueue_->postDelayed(std::move(func), milliSeconds);
}

void CoreRunloop::quit()
{
    DAssert(this == current());
    if (running_)
    {
        running_ = false;
        schedule();
    }
}

void CoreRunloop::postQuit()
{
    DAssert(this == current());
    if (running_)
    {
        post([]() {
            CoreRunloop::current()->running_ = false;
        });
    }

}
void CoreRunloop::run()
{
    DAssert(this == current());
    runLoop();
}

void CoreRunloop::runLoop()
{
    while (running_)
    {
        runOnce();

        if (running_)
        {
            tryIdleWait();
        }
    }
}

void CoreRunloop::runOnce()
{
    DAssert(this == current());
    if (running_)
        processSystemMessage();

    if (running_)
        runTaskQueue();

    if (running_)
        runDelayQueue();
}

void CoreRunloop::tryIdleWait(unsigned int milliSeconds)
{
    DAssert(this == current());
    if (hasWork_) {
        return;
    }
    auto now = TimeTick::now();
    if (!nextDelay_.isNull())
    {
        unsigned int wait = static_cast<unsigned int>((nextDelay_ - now).milliSecs());
        if (wait > 0)
        {
            wait = std::min(wait, milliSeconds);
            idleWait(wait);
        }
    }
    else
    {
        idleWait(milliSeconds);
    }
}

void CoreRunloop::idleWait(unsigned int milliSeconds)
{
    DAssert(this == current());
    ::MsgWaitForMultipleObjectsEx(0, nullptr, milliSeconds, QS_ALLINPUT, MWMO_INPUTAVAILABLE);
}

void CoreRunloop::scheduleWork()
{
    if (hasWork_)
    {
        return;
    }
    hasWork_ = true;
    PostMessageW(msgWindow_, k_MsgLoopWorkMsg, 0, 0);
}

void CoreRunloop::schedule()
{
    PostMessageW(msgWindow_, k_MsgLoopWakeupMsg, 0, 0);
}

std::thread::id CoreRunloop::threadId()
{
    return createThreadId_;
}

std::deque<priv::WrapTask> CoreRunloop::lockSwapTaskQueue()
{
    return taskQueue_->lockSwapTaskQueue();
}

void CoreRunloop::runTaskQueue()
{
    auto now = TimeTick::now();
    std::deque<priv::WrapTask> taskQueue = lockSwapTaskQueue();

    for (auto it = taskQueue.begin(); it != taskQueue.end(); ++it)
    {
        if (!running_)
        {
            break;
        }
        if (it->run.isNull())
        {
            it->invoke();
        }
        else
        {
            if (now >= it->run)
            {
                it->invoke();
                now = TimeTick::now();
            }
            else
            {
                delayQueue_.push(std::move(*it));
            }
        }
    }

    hasWork_ = taskQueue_->size() != 0;

    if (!delayQueue_.empty())
    {
        scheduleDelayed(delayQueue_.top().run);
    }
}

void CoreRunloop::runDelayQueue()
{
    auto now = TimeTick::now();
    while (running_)
    {
        if (delayQueue_.empty() || delayQueue_.top().run > now)
        {
            break;
        }
        auto task = std::move(const_cast<priv::WrapTask&>(delayQueue_.top()));
        delayQueue_.pop();
        task.invoke();
        now = TimeTick::now();
    }

    if (delayQueue_.empty())
    {
        nextDelay_.setZero();
    }
    else
    {
        nextDelay_ = delayQueue_.top().run;
    }
}

void CoreRunloop::scheduleDelayed(TimeTick runTime)
{
    if (scheduledTimerTime_ && scheduledTimerTime_.value() == runTime)
    {
        return;
    }

    auto msecs = (runTime - TimeTick::now()).milliSecs();

    if (msecs <= 0)
    {
        runDelayQueue();
        if (!nextDelay_.isNull())
        {
            runTime = nextDelay_;
            scheduleDelayed(runTime);
        }
    }
    else
    {
        ::SetTimer(msgWindow_, reinterpret_cast<UINT_PTR>(this), (UINT)msecs, nullptr);
        scheduledTimerTime_ = runTime;
    }
}

void CoreRunloop::onSystemTimer()
{
    ::KillTimer(msgWindow_, reinterpret_cast<UINT_PTR>(this));
    scheduledTimerTime_.reset();

    if (delayQueue_.empty())
        return;

    runDelayQueue();
}

void CoreRunloop::processSystemMessage()
{
    MSG msg = { 0 };
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
        {
            this->sysQuitRecivedEvent()(this);
            PostQuitMessage((int)msg.wParam);
            running_ = false;
            break;
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);

        if (!running_)
        {
            break;
        }
    }
}
}
