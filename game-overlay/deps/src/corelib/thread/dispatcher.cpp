#include "stable.h"
#include "corerunloop.h"
#include "corerunloopsafe.h"
#include "dispatcher.h"


namespace Storm
{

    class DispatcherCallback : public Trackable<mt_policy>
    {
        Q_LEAK_DETECTOR(DispatcherCallback);

        WaitableEvent* event_;
        ThreadDispatcherPtr dispatcher_;
        Callback0 function_;
    public:
        DispatcherCallback(ThreadDispatcherPtr dispatcher, const Callback0& function, WaitableEvent* event = nullptr)
            : function_(function)
            , event_(event)
            , dispatcher_(dispatcher)
        {
            connect();
        }

        DispatcherCallback(ThreadDispatcherPtr dispatcher, Callback0&& function, WaitableEvent* event = nullptr)
            : function_(std::move(function))
            , event_(event)
            , dispatcher_(dispatcher)
        {
            connect();
        }

        void onThreadQuit()
        {
            if (event_)
            {
                event_->set();
            }
        }

        inline void connect()
        {
            if (event_)
            {

                dispatcher_->threadStoppingEvent_.add(&DispatcherCallback::onThreadQuit, this);
                dispatcher_->threadStoppedEvent_.add(&DispatcherCallback::onThreadQuit, this);
            }
        }

        ~DispatcherCallback()
        {
            if (event_)
            {
                dispatcher_->threadStoppingEvent_.remove(this);
                dispatcher_->threadStoppedEvent_.remove(this);
            }    
        }

        void run()
        {
            function_();
            if (event_)
            {
                event_->set();
            }
        }

    };


    thread_local ThreadDispatcherPtr ThreadDispatcher::threadDispatcher_ = nullptr;

    ThreadDispatcher::ThreadDispatcher()
    {
    }

    ThreadDispatcher::~ThreadDispatcher()
    {

    }

    void ThreadDispatcher::threadStart(const CoreRunloopSafePtr& runloopSafe)
    {
        std::lock_guard<std::mutex> lock(selfLock_);
        runloopSafe_ = runloopSafe;
        threadDispatcher_ = shared_from_this();
    }

    void ThreadDispatcher::threadStopping()
    {
        std::lock_guard<std::mutex> lock(selfLock_);
        isStopping = true;
        threadStoppingEvent_();
    }

    void ThreadDispatcher::threadStopped()
    {
        std::lock_guard<std::mutex> lock(selfLock_);
        isStopped = true;
        threadDispatcher_ = nullptr;
        threadStoppedEvent_();
    }

    bool ThreadDispatcher::isCurrentThread()
    {
        std::lock_guard<std::mutex> lock(selfLock_);
        return runloopSafe_ && runloopSafe_->runloopUnSafe() == CoreRunloop::current();
    }

    ThreadDispatcherPtr ThreadDispatcher::current()
    {
        return threadDispatcher_;
    }

    bool ThreadDispatcher::_postFun(const Callback0& function)
    {
        std::lock_guard<std::mutex> lock(selfLock_);
        if (!isStopping && runloopSafe_)
        {
            runloopSafe_->post(function);
            return true;
        }
        return false;
    }

    bool ThreadDispatcher::_postFun(Callback0&& function)
    {
        std::lock_guard<std::mutex> lock(selfLock_);
        if (!isStopping && runloopSafe_)
        {
            runloopSafe_->post(std::move(function));
            return true;
        }
        return false;
    }

    void ThreadDispatcher::_sendFun(const Callback0& function)
    {
        if (isCurrentThread())
        {
            function();
        }
        else
        {
            WaitableEvent event;
            std::shared_ptr<DispatcherCallback> callback = std::make_shared<DispatcherCallback>(shared_from_this(), function, &event);
            if (_postFun(Storm::bind(&DispatcherCallback::run, callback)))
            {
                event.wait();
            }
        }
    }

    void ThreadDispatcher::_sendFun(Callback0&& function)
    {
        if (isCurrentThread())
        {
            function();
        }
        else
        {
            WaitableEvent event;
            std::shared_ptr<DispatcherCallback> callback = std::make_shared<DispatcherCallback>(shared_from_this(), std::move(function), &event);
            if (_postFun(Storm::bind(&DispatcherCallback::run, callback)))
            {
                event.wait();
            }
        }
    }

}