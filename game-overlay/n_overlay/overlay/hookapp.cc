#include "stable.h"
#include "session.h"
#include "hookapp.h"
#include "graphics/d3d9hook.h"
#include "graphics/dxgihook.h"
#include "hook/inputhook.h"
#include "hotkey/hotkeycheck.h"

bool g_graphicsOnly = false;
HANDLE g_hookAppThread = nullptr;


DWORD WINAPI HookAppThread(
    _In_ LPVOID
)
{
    OutputDebugStringA("n HookAppThread enter");
    unsigned res = 0;
    __try
    {
        HookApp::instance()->hookThread();
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        res = -1;
        OutputDebugStringA("n HookAppThread exception");
    }

    OutputDebugStringA("n HookAppThread quit");
    return res;
}

DWORD WINAPI hookLoopThread(_In_ LPVOID)
{
    while (!HookApp::instance()->isQuitSet())
    {
        auto window = GetForegroundWindow();
        DWORD processId = 0;
        GetWindowThreadProcessId(window, &processId);
        if (processId != GetCurrentProcessId())
        {
            Sleep(1000);
            continue;
        }

        HookApp::instance()->deferHook();

        Sleep(1000);
    }

    return 0;
}

HookApp::HookApp()
    : hookQuitedEvent_(true)
{
    processPath_ = win_utils::applicationProcPath();
    processName_ = win_utils::applicationProcName();
}

HookApp::~HookApp()
{
    
}

void HookApp::initialize()
{
    MH_Initialize();

    g_hookAppThread = HookApp::instance()->start();
}

void HookApp::uninitialize()
{
    if (g_hookAppThread)
    {
        HookApp::instance()->quit();

        if (WaitForSingleObject(g_hookAppThread, 1000) != WAIT_OBJECT_0)
        {
            TerminateThread(g_hookAppThread, 0);
        }
    }

    MH_Uninitialize();
}

HookApp * HookApp::instance()
{
    static HookApp hookApp;
    return &hookApp;
}

HANDLE HookApp::start()
{
    return ::CreateThread(nullptr, 0, HookAppThread, nullptr, 0, nullptr);

}

void HookApp::quit()
{
    quitFlag_ = true;

    async([this]() {
        std::lock_guard<std::mutex> lock(runloopLock_);
        runloop_->quit();
    });

    hookQuitedEvent_.wait(5000);
}

void HookApp::startHook()
{
    CHECK_THREAD(Threads::HookApp);

    __trace__;

    if (!hookFlag_)
    {
        hookFlag_ = true;
        hookloopThread_ = CreateThread(nullptr, 0, hookLoopThread, nullptr, 0, nullptr);
    }
}

void HookApp::async(const std::function<void()>& task)
{
    std::lock_guard<std::mutex> lock(runloopLock_);
    {
        if (runloop_)
        {
            runloop_->post(task);
        }
    }
}

void HookApp::deferHook()
{
    if (!quitFlag_ && !session::graphicsActive())
    {
        async([this]() { hook(); });
    }
}

void HookApp::hookThread()
{
    LOGGER("n_overlay") << "@trace hook thread start ... ";

    session::setHookAppThreadId(::GetCurrentThreadId());

    {
        std::lock_guard<std::mutex> lock(runloopLock_);
        runloop_.reset(new Storm::CoreRunloop());
    }

    overlay_.reset(new OverlayConnector());
    overlay_->start();

    uiapp_.reset(new UiApp());

    runloop_->post([this]() {
        this->findGameWindow();
    });

    HotkeyCheck::instance()->start();

    runloop_->run();

    HotkeyCheck::instance()->stop();

    {
        std::lock_guard<std::mutex> lock(runloopLock_);
        runloop_.reset(nullptr);
    }

    unhookGraphics();

    overlay_->quit();
    overlay_.reset();

    hookQuitedEvent_.set();

    LOGGER("n_overlay") << "@trace hook thread exit... ";
}

struct FindWindowParam {
    DWORD processId;
    HWND window;
};

BOOL CALLBACK findGraphicsWindow(HWND hwnd, LPARAM lParam)
{
    FindWindowParam* param = (FindWindowParam*)lParam;

    DWORD processId = NULL;

    GetWindowThreadProcessId(hwnd, &processId);

    if (processId != param->processId)
    {
        return TRUE;
    }

    if (!IsWindowVisible(hwnd))
    {
        return TRUE;
    }

    if (GetForegroundWindow() != hwnd)
    {
        return TRUE;
    }

    DWORD styles = (DWORD)GetWindowLongPtr(hwnd, GWL_STYLE);
    if (styles & WS_CHILD)
    {
        return TRUE;
    }

    WCHAR title[MAX_PATH] = { 0 };
    GetWindowTextW(hwnd, title, MAX_PATH);
    if (wcsstr(title, L"Debug") != nullptr)
    {
        return TRUE;
    }

    param->window = hwnd;
    return FALSE;
}

bool HookApp::findGameWindow()
{
    if (session::graphicsWindow())
    {
        return true;
    }

    HWND injectWindow = session::injectWindow();
    if (injectWindow)
    {
        LOGGER("n_overlay") << "setGraphicsWindow by injectWindow: " << injectWindow;
        session::setGraphicsWindow(injectWindow);
    }
    else
    {
        FindWindowParam param = {0};
        param.processId = GetCurrentProcessId();

        EnumWindows(findGraphicsWindow, (LPARAM)&param);

        if (param.window)
        {
            LOGGER("n_overlay") << "setGraphicsWindow by enum: " << param.window;
            session::setGraphicsWindow(param.window);
        }
    }

    std::cout << __FUNCTION__ << ", injectWindow: " << session::injectWindow() << std::endl;
    std::cout << __FUNCTION__ << ", graphicsWindow: " << session::graphicsWindow() << std::endl;

    return !!session::graphicsWindow();
}

void HookApp::hook()
{
    __trace__;
    if (!findGameWindow())
    {
        return;
    }

    if (!g_graphicsOnly)
    {
        if (!hookInput())
        {
            return;
        }

        if (!hookWindow())
        {
            return;
        }
    }

    hookGraphics();
}

bool HookApp::hookWindow()
{
    DAssert(session::graphicsWindow());

    return uiapp_->trySetupGraphicsWindow(session::graphicsWindow());
}

void HookApp::unhookGraphics()
{
    if (session::d3d9Hooked())
    {
        session::d3d9Hook()->unhook();
        session::clearD3d9Hook();
    }

    if (session::dxgiHooked())
    {
        session::dxgiHook()->unhook();
        session::clearDxgiHook();
    }
}

void HookApp::hookGraphics()
{
    if (!session::d3d9Hooked())
    {
        hookD3d9();
    }

    if (!session::dxgiHooked())
    {
        hookDXGI();
    }
}

bool HookApp::hookInput()
{
    if (!session::inputHooked())
    {
        session::tryInputHook();

        std::cout << __FUNCTION__ << ", " << session::inputHooked() << std::endl;
    }

    return session::inputHooked();
}

bool HookApp::hookD3d9()
{
    if (!session::d3d9Hooked())
    {
        session::tryD3d9Hook();

        std::cout << __FUNCTION__ << ", " << session::d3d9Hooked() << std::endl;
    }

    return session::d3d9Hooked();
}

bool HookApp::hookDXGI()
{
    if (!session::dxgiHooked())
    {
        session::tryDxgiHook();
    }

    std::cout << __FUNCTION__ << ", " << session::dxgiHooked() << std::endl;
    return session::dxgiHooked();
}

