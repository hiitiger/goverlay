#include "stable.h"
#include "weakobjectptr.h"
#include "object.h"
#include "thread/corerunloop.h"

namespace Storm
{
    CoreObject::CoreObject(CoreObject* parent )
        : parent_(parent)
        , weakGuard_(new WeakGuard(this))
    {
        if (parent_)
        {
            parent_->children_.push_back(this);
        }
    }

    CoreObject::~CoreObject()
    {
        if (weakGuard_->hasPtr())
        {
            weakGuard_->clearGuardObject(this);
        }
        weakGuard_.reset(nullptr);

        deleteChildren();

        setParent(nullptr);

        this->destroyEvent_(this);
    }

    void CoreObject::setParent(CoreObject* parent)
    {
        if (parent != parent_ && parent != this)
        {
            if (parent_)
            {
                auto it = std::find(parent_->children_.begin(), parent_->children_.end(), this);
                if (it != parent_->children_.end())
                {
                    parent_->children_.erase(it);
                }
                parent_ = nullptr;
            }

            if (parent)
            {
                parent_ = parent;
                parent_->children_.push_back(this);
            }
        }
    }
    CoreObject* CoreObject::parent() const
    {
        return parent_;
    }

    const std::vector<CoreObject*>& CoreObject::children() const
    {
        return children_;
    }

    void CoreObject::deleteLater()
    {
        CoreRunloop::current()->post([weakSelf = weakSelf()]() {
            if (weakSelf)
            {
                delete weakSelf;
            }
        });
    }

    WeakObjectPtr<CoreObject> CoreObject::weakSelf()
    {
        return this;
    }

    WeakGuard *CoreObject::getGuard() const
    {
        return weakGuard_.get();
    }

    void CoreObject::deleteChildren()
    {
        for (auto& child : children_)
        {
            child->parent_ = nullptr;
            delete child;
            child = nullptr;
        }

        children_.clear();
    }
}