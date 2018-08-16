#include "stable.h"
#include "object.h"
#include "weakobjectptr.h"

namespace Storm {


    std::multimap<CoreObject*, CoreObject**> WeakGuard::ptrmap_;
    std::mutex WeakGuard::objmapLock_;

    WeakGuard::WeakGuard(CoreObject* obj)
        : obj_(obj)
        , hasGuardPtr_(false)
    {
        ;
    }
    void WeakGuard::clearGuardObject(CoreObject* obj)
    {
        clearObj(obj);
    }

    void WeakGuard::setHasPtr(bool i)
    {
        hasGuardPtr_ = i;
    }

    bool WeakGuard::hasPtr() const
    {
        return hasGuardPtr_;
    }

    void WeakGuard::setsObj(CoreObject* obj)
    {
        obj_ = obj;
    }

    void WeakGuard::addObj(CoreObject** objaddr)
    {
        if (*objaddr)
        {
            std::lock_guard<std::mutex> lock(objmapLock_);
            ptrmap_.insert(std::pair<CoreObject *, CoreObject **>(*objaddr, objaddr));
            (*objaddr)->getGuard()->setHasPtr(true);
        }
    }

    void WeakGuard::removeObj(CoreObject** objaddr)
    {
        if (*objaddr)
        {
            std::lock_guard<std::mutex> lock(objmapLock_);

            std::multimap< CoreObject*, CoreObject**>::iterator it = ptrmap_.find(*objaddr);
            for (; it != ptrmap_.end(); )
            {
                if (it->first == *objaddr && it->second == objaddr) //remove just one
                {
                    ptrmap_.erase(it++);
                    break;
                }
                else
                {
                    ++it;
                }
            }
        }
    }

    void WeakGuard::clearObj(CoreObject* obj)
    {
        std::lock_guard<std::mutex> lock(objmapLock_);

        std::multimap< CoreObject*, CoreObject**>::iterator it = ptrmap_.find(obj);

        for (; it != ptrmap_.end();)
        {
            if (it->first == obj)   //remove all
            {
                *(it->second) = 0;
                ptrmap_.erase(it++);
            }
            else
            {
                ++it;
            }
        }
    }

}

