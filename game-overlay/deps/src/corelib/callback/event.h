#pragma once

namespace Storm
{
    namespace priv
    {
        class dummy_lock
        {
        public:
            void lock() {}
            void unlock() {}
        };

    }

    typedef priv::dummy_lock st_policy;
    typedef std::mutex mt_policy;

    typedef mt_policy def_thread_policy;

    template<class ThreadPolicy>
    class Trackable;

    template <class, class>
    class Event;

    class ConnectionConextBase;

    template<class T, class ThreadPolicy>
    class ConnectionContext;

    template<class ThreadPolicy>
    class EventBase
    {
        STORM_NONCOPYABLE(EventBase);
    public:
        EventBase() = default;
        virtual ~EventBase() {}
        virtual void connectionDisconnect(ConnectionConextBase*) = 0;
        virtual void trackableExpired(Trackable<ThreadPolicy>* object) = 0;
    };

    class ConnectionConextBase : public std::enable_shared_from_this<ConnectionConextBase>
    {
    protected:
        std::atomic<bool> connected_ = true;
    public:
        ConnectionConextBase() : connected_(true){}
        virtual ~ConnectionConextBase() {}

        virtual void disconnectSelf() = 0;
        virtual void trackableExpired() = 0;
        virtual void lock() = 0;
        virtual void unlock() = 0;

        bool isConnected() const { return connected_; }

        void disconnect()
        {
            //disconnect from event 
            //so this will require lock
            disconnectSelf();
            connected_ = false;
        }

        void disconnected()
        {
            std::lock_guard<ConnectionConextBase> lock(*this);
            connected_ = false;
        }

        void nolockDisconnected()
        {
            connected_ = false;
        }
    };


    template<class R, class ...A, class ThreadPolicy>
    class ConnectionContext<R(A...), ThreadPolicy> : public ConnectionConextBase
    {
        EventBase<ThreadPolicy>* event_;
        Delegate<R(A...)> delegate_;
        Trackable<ThreadPolicy>* contextObject_;
        ThreadPolicy lock_;
    public:
        ConnectionContext(EventBase<ThreadPolicy>* e, const Delegate<R(A...)>& d, Trackable<ThreadPolicy>* t)
            : event_(e), delegate_(d), contextObject_(t)
        {
            
        }

        ConnectionContext(EventBase<ThreadPolicy>* e, Delegate<R(A...)>&& d, Trackable<ThreadPolicy>* t)
            : event_(e), delegate_(std::move(d)), contextObject_(t)
        {

        }

        void disconnectSelf() override
        {
            std::lock_guard<ConnectionContext> lock(*this);
            if (isConnected())
            {
                nolockDisconnected();
                event_->connectionDisconnect(this);
                event_ = nullptr;
                contextObject_ = nullptr;
            }
        }

        void trackableExpired() override
        {
            std::lock_guard<ConnectionContext> lock(*this);
            if (isConnected())
            {
                nolockDisconnected();
                event_->trackableExpired(contextObject_);
                event_ = nullptr;
                contextObject_ = nullptr;
            }
        }

        void lock() override
        {
            lock_.lock();
        }

        void unlock() override
        {
            lock_.unlock();
        }

        void invoke(A ...args)
        {
            // here we don't need lock
            if (isConnected())
            {
                delegate_(std::forward<A>(args)...);
            }
        }

        auto getDelegate() const
        {
            return delegate_;
        }

        Trackable<ThreadPolicy>* getTrackable()
        {
            std::lock_guard<ConnectionContext> lock(*this);
            return contextObject_;
        }

        Trackable<ThreadPolicy>* nolockGetTrackable() const
        {
            return contextObject_;
        }
    };

    class Connection
    {
    protected:  
        std::weak_ptr<ConnectionConextBase> context_;
    public:
        Connection(const std::weak_ptr<ConnectionConextBase>& d)
            :context_(std::weak_ptr<ConnectionConextBase>(d))
        {
            
        }

        void disconnect()
        {
            if (auto context = context_.lock())
            {
                context->disconnect();
            }
        }

    };

    template<class ThreadPolicy = def_thread_policy>
    class Trackable 
    {
        STORM_NONCOPYABLE(Trackable);

        ThreadPolicy trackableLock_;
        std::set<std::shared_ptr<ConnectionConextBase>> connections_;
    public:
        Trackable() = default;
        virtual ~Trackable()
        {
            removeAll();
        }

        void connectionAdded(const std::shared_ptr<ConnectionConextBase> & conn)
        {
            std::lock_guard<ThreadPolicy> lock(trackableLock_);
            connections_.insert(conn);
        }

        void connectionRemoved(const std::shared_ptr<ConnectionConextBase> & conn)
        {
            std::lock_guard<ThreadPolicy> lock(trackableLock_);
            auto it = connections_.find(conn);
            if (it != connections_.end())
            {
                connections_.erase(it);
            }
        }

        void removeAll()
        {
            std::lock_guard<ThreadPolicy> lock(trackableLock_);
            for (auto it = connections_.begin(); it != connections_.end(); ++it)
            {
                (*it)->trackableExpired();
            }
            connections_.clear();
        }

    };

    template <class R, class ...A, class ThreadPolicy>
    class Event<R(A ...), ThreadPolicy> : public EventBase<ThreadPolicy>
    {
        using Delegate = Delegate<R(A ...)>;
        using ConnectionContext = ConnectionContext<R(A ...), ThreadPolicy>;
        std::shared_ptr<std::vector<std::shared_ptr<ConnectionContext>>> connections_;
        ThreadPolicy lock_;

        void compactConnections()
        {
            //make sure operation on connections is safe
            if (!connections_.unique())
            {
                connections_.reset(new std::vector<std::shared_ptr<ConnectionContext>>(*connections_));
            }
        }

        Connection addConnection(const std::shared_ptr<ConnectionContext>& conn, Trackable<ThreadPolicy>* trackable = nullptr)
        {
            compactConnections();

            connections_->emplace_back(conn);
            if (trackable)
            {
                trackable->connectionAdded(conn);
            }
            return Connection(conn);
        }

        void connectionDisconnect(ConnectionConextBase* conn) override
        {
            std::lock_guard<ThreadPolicy> lock(lock_);
            compactConnections();

            auto it = connections_->begin();
            for (; it != connections_->end();)
            {
                if ((*it).get() == conn)
                {
                    auto conn = *it;
                    auto object = conn->nolockGetTrackable();
                    connections_->erase(it);
                    if (object)
                    {
                        object->connectionRemoved(conn);
                    }
                    return;
                }
                else
                {
                    ++it;
                }
            }
        }

        void trackableExpired(Trackable<ThreadPolicy>* object) override
        {
            std::lock_guard<ThreadPolicy> lock(lock_);
            compactConnections();

            auto it = connections_->begin();
            for (; it != connections_->end();)
            {
                if ((*it)->nolockGetTrackable() == object)
                {
                    auto conn = *it;
                    it = connections_->erase(it);
                    continue;
                }
                else
                {
                    ++it;
                }
            }
        }

    public:
        Event() : connections_(std::make_shared<std::vector<std::shared_ptr<ConnectionContext>>>())
        {
            
        }

        ~Event()
        {
            removeAll();
        }

        Connection add(Delegate&& delegate)
        {
            std::lock_guard<ThreadPolicy> lock(lock_);
            return addConnection(std::make_shared<ConnectionContext>(this, std::move(delegate), nullptr));
        }

        Connection add(const Delegate& delegate)
        {
            std::lock_guard<ThreadPolicy> lock(lock_);
            return addConnection(std::make_shared<ConnectionContext>(this, delegate, nullptr));
        }

        template<class C, class O, typename = std::enable_if_t<std::is_base_of<Trackable<ThreadPolicy>, std::remove_cv_t<std::remove_reference_t<O>>>::value, bool>>
        Connection add(R(C::*function)(A ...), O* object)
        {
            std::lock_guard<ThreadPolicy> lock(lock_);
            return addConnection(std::make_shared<ConnectionContext>(this, Storm::delegate(function, object), object), object);
        }

        template<class O, typename = std::enable_if_t<std::is_base_of<Trackable<ThreadPolicy>, std::remove_cv_t<std::remove_reference_t<O>>>::value, bool>>
        Connection add(const Delegate& delegate, O* object)
        {
            std::lock_guard<ThreadPolicy> lock(lock_);
            return addConnection(std::make_shared<ConnectionContext>(this, delegate, object), object);
        }

        template<class O, typename = std::enable_if_t<std::is_base_of<Trackable<ThreadPolicy>, std::remove_cv_t<std::remove_reference_t<O>>>::value, bool>>
        Connection add(Delegate&& delegate, O* object)
        {
            std::lock_guard<ThreadPolicy> lock(lock_);
            return addConnection(std::make_shared<ConnectionContext>(this, std::move(delegate), object), object);
        }

        void remove(Trackable<ThreadPolicy>* object)
        {
            std::lock_guard<ThreadPolicy> lock(lock_);
            compactConnections();

            auto it = connections_->begin();
            for (; it != connections_->end();)
            {
                if ((*it)->getTrackable() == object)
                {
                    auto conn = *it;
                    conn->disconnected();
                    it = connections_->erase(it);
                    object->connectionRemoved(conn);
                    continue;
                }
                else
                {
                    ++it;
                }
            }
        }

        void remove(const Delegate& delegate)
        {
            std::lock_guard<ThreadPolicy> lock(lock_);
            compactConnections();

            auto it = connections_->begin();
            for (; it != connections_->end();)
            {
                if ((*it)->getDelegate() == delegate)
                {
                    auto conn = *it;
                    auto object = conn->getTrackable();
                    conn->disconnected();
                    it = connections_->erase(it);
                    if (object)
                    {
                        object->connectionRemoved(conn);
                    }
                    continue;
                }
                else
                {
                    ++it;
                }
            }
        }

        void removeAll()
        {
            std::lock_guard<ThreadPolicy> lock(lock_);

            for (auto it = connections_->begin(); it != connections_->end(); ++it)
            {
                auto conn = *it;
                conn->disconnected();
                auto object = (*it)->getTrackable();
                if (object)
                {
                    object->connectionRemoved(conn);
                }
            }
            connections_->clear();
        }

        void operator +=(const Delegate& delegate)
        {
            add(delegate);
        }

        void operator +=(Delegate&& delegate)
        {
            add(std::move(delegate));
        }

        void operator -=(const Delegate& delegate)
        {
            remove(delegate);
        }

        void emit(arg_traits_t<A>...args)
        {
            //so we don't need recursive lock
            std::shared_ptr<std::vector<std::shared_ptr<ConnectionContext>>> connections;
            {
                std::lock_guard<ThreadPolicy> lock(lock_);
                connections = connections_;
            }
            for (auto conn : *connections)
            {
                conn->invoke(arg_traits_t<A>(args)...);
            }
        }

        void operator()(A ...args)
        {
            return (*this).emit(std::forward<A>(args)...);
        }
    };

    template<class S, class ThreadPolicy = def_thread_policy>
    class Event : public Event<S, ThreadPolicy>
    {
    };

}
