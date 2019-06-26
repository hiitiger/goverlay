#pragma once



//////////////////////////////////////////////////////////////////////////
template<class T>
class LeakObjectDetect
{
public:
    LeakObjectDetect()
    {
        (getCounter().numObjects) += 1;
      //  std::cout << getLeakedObjectClassName() << ", count:" << getCounter().numObjects << std::endl;
    }

    LeakObjectDetect(const LeakObjectDetect&)
    {
        (getCounter().numObjects) += 1;
     //   std::cout << getLeakedObjectClassName() << ", count:" << getCounter().numObjects << std::endl;
    }


    ~LeakObjectDetect()
    {
        (getCounter().numObjects) -= 1;
     //   std::cout << getLeakedObjectClassName() << ", count:" << getCounter().numObjects << std::endl;
    }

    static const char* getLeakedObjectClassName()
    {
        return T::getLeakedObjectClassName();
    }


    class LeakCounter
    {
    public:
        LeakCounter() {}

        ~LeakCounter()
        {
            if (numObjects > 0)
            {
                std::cout << "*** Leaked objects detected: " << numObjects << " instance(s) of class " << getLeakedObjectClassName() << std::endl;
                DAssert(false);
            }
        }

        std::atomic<int> numObjects = 0;
    };


    static LeakCounter& getCounter() {
        static LeakCounter _counter;
        return _counter;
    }

};

// #define Q_ENABLE_LEAK_DETECT

#ifdef Q_ENABLE_LEAK_DETECT
    #define Q_LEAK_DETECTOR(OwnerClass) \
        friend class LeakObjectDetect<OwnerClass>; \
        static const char* getLeakedObjectClassName() { return #OwnerClass; } \
        LeakObjectDetect<OwnerClass> Leak_##OwnerClass;
#else
    #define Q_LEAK_DETECTOR(OwnerClass) 
#endif

