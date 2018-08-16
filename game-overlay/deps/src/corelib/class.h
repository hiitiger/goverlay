#pragma once


namespace Storm {
    class CoreObject;
    template<class> class WeakObjectPtr;

    class PerfCounter;
    class TimeTick;
    class DateTime;

    class Buffer;
    template<class>         class Array;
    template<class, class>  class KVPair;
    template<class, class>  class Skiplist;
    template<class, class>  class TinyMap;
    template<class, class>  class OrderedMap;
    template<class>         class TinySet;
    template<class>         class SyncQueue;

    class WaitableEvent;
    class Lockable;
    class DummyLock;
    class CSLock;
    class ScopeLovkV1;
    class RWLock;
    class ScopedReadLock;
    class ScopedWriteLock;

    class CoreRunloop;
    class CoreRunloopSafe;
    typedef std::shared_ptr<CoreRunloopSafe> CoreRunloopSafePtr;

    class CoreThread;

    class ThreadDispatcher;
    typedef std::shared_ptr<ThreadDispatcher> ThreadDispatcherPtr;

    class WorkerPool;
    class Timer;

    class TextStream;
    class File;

    class XmlElement;
    class XmlDocument;

    template<class>         class Callback;
    template<class T>       using Delegate = Callback<T>;


    class ConnectionConextBase;
    class Connection;
    template<class>         class EventBase;
    template<class>         class Trackable;
    template<class, class>  class ConnectionContext;
    template<class, class>  class Event;

    class Throttle;

    template<bool> class Snowflake;

    namespace priv
    {
        class WrapTask;

        class RunloopTaskQueue;
        typedef std::shared_ptr<RunloopTaskQueue> RunloopTaskQueuePtr;
    }
}

typedef Storm::Callback<void()>  Callback0;
