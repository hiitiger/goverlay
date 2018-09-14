#include "stable.h"
#include "graphics/d3d9hook.h"
#include "graphics/dxgihook.h"
#include "hook/inputhook.h"
#include "hookapp.h"
#include "overlay.h"
#include "session.h"

namespace session
{
std::atomic<HWND> g_injectWindow = nullptr;
std::atomic<HWND> g_graphicsWindow = nullptr;

std::uint32_t hookAppThreadId_ = 0;
std::uint32_t windowThreadId_ = 0;
std::uint32_t graphicsThreadId_ = 0;

overlay_game::D3d9HookInfo d3d9HookInfo_;
overlay_game::DxgiHookInfo dxgiHookInfo_;

std::atomic<bool> d3d9Hooked_ = false;
std::atomic<bool> dxgiHooked_ = false;

std::unique_ptr<D3d9Hook> d3d9Hook_;
std::unique_ptr<DXGIHook> dxgiHook_;

std::atomic<bool> inputHooked_ = false;
std::unique_ptr<InputHook> inputHook_;

std::atomic<bool> graphicsActive_ = false;
std::atomic<bool> overlayConnected_ = false;
std::atomic<bool> overlayEnabled_ = true;

HMODULE hModuleD3dCompiler47_ = nullptr;


D3d9Hook *d3d9Hook()
{
    return d3d9Hook_.get();
}

DXGIHook *dxgiHook()
{
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

void tryD3d9Hook()
{
    CHECK_THREAD(Threads::HookApp);

    d3d9Hook_ = std::make_unique<D3d9Hook>();

    if (d3d9Hook_->hook())
    {
        d3d9Hooked_ = true;
    }
    else
    {
        d3d9Hook_.reset();
    }
}

void tryDxgiHook()
{
    CHECK_THREAD(Threads::HookApp);
    dxgiHook_ = std::make_unique<DXGIHook>();

    if (dxgiHook_->hook())
    {
        dxgiHooked_ = true;
    }
    else
    {
        dxgiHook_.reset();
    }
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

InputHook *inputHook()
{
    return inputHook_.get();
}

bool inputHooked()
{
    return inputHooked_;
}

void tryInputHook()
{
    CHECK_THREAD(Threads::HookApp);

    inputHook_ = std::make_unique<InputHook>();
    if (inputHook_->hook())
    {
        inputHooked_ = true;
    }
    else
    {
        inputHook_.reset();
    }
}

HMODULE loadModuleD3dCompiler47()
{
    CHECK_THREAD(Threads::Graphics);

    if (!hModuleD3dCompiler47_)
    {
        hModuleD3dCompiler47_ = LoadLibraryW(L"d3dcompiler_47.dll");
        if (!hModuleD3dCompiler47_)
        {
            hModuleD3dCompiler47_ = LoadLibraryW(HookApp::instance()->overlayConnector()->mainProcessDir().c_str());
        }
    }

    return hModuleD3dCompiler47_;
}

overlay_game::D3d9HookInfo &d3d9HookInfo()
{
    return d3d9HookInfo_;
}

overlay_game::DxgiHookInfo &dxgiHookInfo()
{
    return dxgiHookInfo_;
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

void setGraphicsActive(bool active)
{
    graphicsActive_ = active;
}

bool graphicsActive()
{
    return graphicsActive_;
}

void setOverlayConnected(bool value)
{
    overlayConnected_ = value;
}

bool overlayConnected()
{
    return overlayConnected_;
}

void setOverlayEnabled(bool value)
{
    overlayEnabled_ = value;
}

bool overlayEnabled()
{
    return overlayEnabled_;
}
} // namespace session