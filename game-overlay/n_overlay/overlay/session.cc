#include "stable.h"
#include "graphics/d3d9hook.h"
#include "graphics/dxgihook.h"
#include "hook/inputhook.h"
#include "session.h"

namespace session 
{

    HWND g_injectWindow = nullptr;
    std::atomic<HWND> g_graphicsWindow = nullptr;

    std::uint32_t hookAppThreadId_ = 0;
    std::uint32_t windowThreadId_ = 0;
    std::uint32_t graphicsThreadId_ = 0;

    D3d9HookInfo g_d3d9HookInfo;
    DxgiHookInfo g_dxgiHookInfo;


    std::atomic<bool> d3d9Hooked_ = false;
    std::atomic<bool> dxgiHooked_ = false;

    std::unique_ptr<D3d9Hook> d3d9Hook_;
    std::unique_ptr<DXGIHook> dxgiHook_;

    std::atomic<bool> inputHooked_ = false;
    std::unique_ptr<InputHook> inputHook_;


    D3d9Hook* d3d9Hook()
    {
        DAssert(d3d9Hook_);

        return d3d9Hook_.get();
    }

    DXGIHook* dxgiHook()
    {
        DAssert(dxgiHook_);

        return dxgiHook_.get();
    }

    bool d3d9Hooked()
    {
        return d3d9Hooked_;
    }

    bool dxgiHooked()
    {
        return dxgiHooked_;
    }

    void saveD3d9Hook(std::unique_ptr<D3d9Hook>&& h)
    {
        CHECK_THREAD(Threads::HookApp);

        d3d9Hook_ = std::move(h);
        d3d9Hooked_ = true;
    }

    void saveDxgiHook(std::unique_ptr<DXGIHook>&& h)
    {
        CHECK_THREAD(Threads::HookApp);

        dxgiHook_ = std::move(h);
        dxgiHooked_ = true;
    }

    void clearD3d9Hook()
    {
        CHECK_THREAD(Threads::HookApp);

        d3d9Hook_ = nullptr;
        d3d9Hooked_ = false;
    }

    void clearDxgiHook()
    {
        CHECK_THREAD(Threads::HookApp);

        dxgiHook_ = nullptr;
        dxgiHooked_ = false;
    }

    bool inputHooked()
    {
        return inputHooked_;
    }

    void saveInputHook(std::unique_ptr<InputHook>&& h)
    {
        CHECK_THREAD(Threads::HookApp);

        inputHooked_ = true;
        inputHook_ = std::move(h);
    }

    D3d9HookInfo& d3d9HookInfo()
    {
        return g_d3d9HookInfo;
    }

    DxgiHookInfo& dxgiHookInfo()
    {
        return g_dxgiHookInfo;
    }

    std::uint32_t hookAppThreadId()
    {
        return hookAppThreadId_;
    }

    void setHookAppThreadId(DWORD id)
    {
        hookAppThreadId_ = id;
    }

    std::uint32_t windowThreadId()
    {
        return windowThreadId_;
    }

    void setWindowThreadId(DWORD id)
    {
        windowThreadId_ = id;
    }

    std::uint32_t graphicsThreadId()
    {
        return graphicsThreadId_;
    }

    void setGraphicsThreadId(DWORD id)
    {
        graphicsThreadId_ = id;
    }

    void setInjectWindow(HWND window)
    {
        g_injectWindow = window;
    }

    HWND injectWindow()
    {
        return g_injectWindow;
    }

    void setGraphicsWindow(HWND window)
    {
        g_graphicsWindow = window;
    }

    HWND graphicsWindow()
    {
        return g_graphicsWindow;
    }

}