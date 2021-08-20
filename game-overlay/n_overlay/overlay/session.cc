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
std::atomic<bool> windowed_ = false;

std::atomic<bool> overlayConnected_ = false;
std::atomic<bool> overlayEnabled_ = true;
// did not really need this to be atomic
bool overlayVisible_ = true;

HMODULE hModuleD3dCompiler47_ = nullptr;
HMODULE hModuleD3dx9_ = nullptr;

std::atomic<bool> hasIGO_ = false;

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

bool tryD3d9Hook()
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
    return d3d9Hooked_;
}

bool tryDxgiHook()
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

    return dxgiHooked_;
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

bool tryInputHook()
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

    return inputHooked_;
}

HMODULE loadModuleD3dCompiler47()
{
    CHECK_THREAD(Threads::Graphics);

    if (!hModuleD3dCompiler47_)
    {
        hModuleD3dCompiler47_ = LoadLibraryW(L"d3dcompiler_47.dll");
        if (!hModuleD3dCompiler47_)
        {
            hModuleD3dCompiler47_ = LoadLibraryW(HookApp::instance()->overlayConnector()->d3dcompiler47Path().c_str());
        }
    }

    return hModuleD3dCompiler47_;
}

HMODULE loadD3dx9()
{
    CHECK_THREAD(Threads::Graphics);
    if (!hModuleD3dx9_)
    {
        hModuleD3dx9_ = LoadLibraryW(L"d3dx9_43.dll");
        if (!hModuleD3dx9_)
        {
            hModuleD3dx9_ = LoadLibraryW(HookApp::instance()->overlayConnector()->d3dx9Path().c_str());
        }
    }

    return hModuleD3dx9_;
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

    setWindowThreadId(GetWindowThreadProcessId(window, nullptr));
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

void setIsWindowed(bool windowed)
{
    windowed_ = windowed;
}

bool isWindowed()
{
    return windowed_;
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

void setOverlayVisible(bool v)
{
    overlayVisible_ = v;
}

bool overlayVisible()
{
    return overlayVisible_;
}

bool hasIGO()
{
    return hasIGO_;
}

void checkIGO()
{
#if _WIN64
    hasIGO_ = GetModuleHandleW(L"igo64.dll") != NULL;
#else
    hasIGO_ = GetModuleHandleW(L"igo32.dll") != NULL;
#endif // #if _WIN64
}

} // namespace session