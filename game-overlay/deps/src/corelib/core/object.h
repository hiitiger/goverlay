#pragma once

namespace Storm
{
    class CoreObject
    {
        STORM_NONCOPYABLE(CoreObject);
    private:
        friend class WeakGuard;
        std::unique_ptr<WeakGuard> weakGuard_;

        CoreObject* parent_ = nullptr;
        std::vector<CoreObject*> children_;

        Event<void(CoreObject*)> destroyEvent_;

    public:
        explicit CoreObject(CoreObject* parent = nullptr);

        virtual ~CoreObject();

        void setParent(CoreObject* parent);

        CoreObject* parent() const;

        const std::vector<CoreObject*>& children() const;

        void deleteLater();
  
        Event<void(CoreObject*)>& destroyEvent() { return destroyEvent_; }

    protected:
        WeakObjectPtr<CoreObject> weakSelf();

    private:
        WeakGuard *getGuard() const;
        void deleteChildren();
    };
}
