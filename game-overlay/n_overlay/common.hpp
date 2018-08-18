#pragma once
#include "overlay/session.h"

struct IHookModule
{
    virtual ~IHookModule() {;}
    virtual bool hook() = 0;
    virtual void unhook() = 0;
};


enum class Graphics
{
    D3d9 = 1,
    DXGI = 2,
    OpenGL = 8,
};


enum class Threads
{
    HookApp = 1,
    Graphics = 2,
    Window = 3,
};


inline bool checkThread(Threads type)
{
    std::uint32_t required = 0;
    switch (type)
    {
    case Threads::HookApp:
        required = session::hookAppThreadId();
        break;
    case Threads::Graphics:
        required = session::graphicsThreadId();
        break;
    case Threads::Window:
        required = session::windowThreadId();
        break;
    default:
        break;
    }

    return ::GetCurrentProcessId() == required;

}

#define CHECK_THREAD(type) \
do \
{\
    assert(checkThread(type));\
} while (0);
