#pragma once


namespace Storm
{

class ThreadDispatcher : public std::enable_shared_from_this<ThreadDispatcher>
{
    STORM_NONCOPYABLE(ThreadDispatcher);
    Q_LEAK_DETECTOR(ThreadDispatcher);

protected:
    friend class CoreThread;
    friend class DispatcherCallback;

    static thread_local  ThreadDispatcherPtr threadDispatcher_;

    std::mutex selfLock_;
    bool isStopping = false;
    bool isStopped = false;

    CoreRunloopSafePtr runloopSafe_;

    void threadStart(const CoreRunloopSafePtr&);
    void threadStopping();
    void threadStopped();

    bool isCurrentThread();

    Event<void(), mt_policy> threadStoppingEvent_;
    Event<void(), mt_policy> threadStoppedEvent_;
public:
    ThreadDispatcher();
    ~ThreadDispatcher();
    static ThreadDispatcherPtr current();

    template<class Fn, typename = std::enable_if<!std::is_member_function_pointer<Fn>::value, bool>::type, class ...A>
    void invokeAsync(Fn&& function, A&&... args)
    {
        Callback0 callback = Storm::bind(std::forward<Fn>(function), std::forward<A>(args)...);
        _postFun(std::move(callback));
    }

    template<class Fn, typename = std::enable_if<std::is_member_function_pointer<Fn>::value, bool>::type, class C, class ...A >
    void invokeAsync(Fn&& function, C* object, A&&... args)
    {
        Callback0 callback = Storm::bind(std::forward<Fn>(function), object, std::forward<A>(args)...);
        _postFun(std::move(callback));
    }

    template<class Fn, typename = std::enable_if<std::is_member_function_pointer<Fn>::value, bool>::type , class C, class ...A>
    void invokeAsync(Fn&& function, std::shared_ptr<C>& object, A&&... args)
    {
        Callback0 callback = Storm::bind(std::forward<Fn>(function), object, std::forward<A>(args)...);
        _postFun(std::move(callback));
    }

    template<class Owner, class S, class ...A >
    void emitAsync(Owner* object, Event<S>* event,  A&&... args)
    {
        Callback0 eventCallback = Storm::bind(&Event<S>::operator(), event, std::forward<A>(args)...);

        Callback0 callback = [=]() {
            if (object)
            {
                eventCallback();
            }
        };
        _postFun(std::move(callback));
    }

    template<class Owner, class S, class ...A >
    void emitAsync(const std::shared_ptr<Owner>& object, Event<S>* event, A&&... args)
    {
        std::shared_ptr<Owner> object_ = object;
        Callback0 eventCallback = Storm::bind(&Event<S>::operator(), event, std::forward<A>(args)...);

        Callback0 callback = [=]() {
            if (object_)
            {
                eventCallback();
            }
        };
        _postFun(std::move(callback));
    }

    template<class Fn, typename = std::enable_if<!std::is_member_function_pointer<Fn>::value, bool>::type, class ...A >
    void invoke(Fn&& function, A&&... args)
    {
        Callback0 callback = Storm::bind(std::forward<Fn>(function), std::forward<A>(args)...);
        _sendFun(std::move(callback));
    }

    template<class Fn, typename = std::enable_if<std::is_member_function_pointer<Fn>::value, bool>::type , class C, class ...A>
    void invoke(Fn&& function, C* object, A&&... args)
    {
        Callback0 callback = Storm::bind(std::forward<Fn>(function), object, std::forward<A>(args)...);
        _sendFun(std::move(callback));
    }

    template<class Fn, typename = std::enable_if<std::is_member_function_pointer<Fn>::value, bool>::type,  class C, class ...A>
    void invoke(Fn&& function, std::shared_ptr<C>& object, A&&... args)
    {
        Callback0 callback = Storm::bind(std::forward<Fn>(function), object, std::forward<A>(args)...);
        _sendFun(std::move(callback));
    }

    void invoke(const Callback0& function)
    {
        _sendFun(function);
    }
    void invoke(Callback0&& function)
    {
        _sendFun(std::move(function));
    }
    void invokeAsync(const Callback0& function)
    {
        _postFun(function);
    }
    void invokeAsync(Callback0&& function)
    {
        _postFun(std::move(function));
    }

protected:
    bool _postFun(const Callback0& function);
    bool _postFun(Callback0&& function);
    void _sendFun(const Callback0& function);
    void _sendFun(Callback0&& function);
};

}
