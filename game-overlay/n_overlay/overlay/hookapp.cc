#include "stable.h"
#include "session.h"
#include "hookapp.h"
#include "graphics/d3d9hook.h"
#include "graphics/dxgihook.h"


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
    while (HookApp::instance()->isQuitSet())
    {
        auto window = GetForegroundWindow();
        DWORD processId = 0;
        GetWindowThreadProcessId(window, &processId);
        if (processId != GetCurrentProcessId())
        {
            Sleep(1000);
            continue;
        }

        HookApp::instance()->tryHookInput();
        HookApp::instance()->tryHookGraphics();

        Sleep(1000);
    }

    return 0;
}

HookApp::HookApp()
{
    processPath_ = win_utils::applicationProcPath();
    processName_ = win_utils::applicationProcName();
}

HookApp::~HookApp()
{
    
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

    wait_.wait();
}

void HookApp::startHook()
{
    CHECK_THREAD(Threads::HookApp);

    hookFlag_ = true;

    hookloopThread_ = CreateThread(nullptr, 0, hookLoopThread, nullptr, 0, nullptr);
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

void HookApp::hookThread()
{
    LOGGER("n_overlay") << "@trace hook thread start ... ";

    session::setHookAppThreadId(::GetCurrentThreadId());

    {
        std::lock_guard<std::mutex> lock(runloopLock_);
        runloop_.reset(new Storm::CoreRunloop());
    }

    overlay_.reset(new overlay::OverlayConnector());
    overlay_->start();

    runloop_->run();

    {
        std::lock_guard<std::mutex> lock(runloopLock_);
        runloop_.reset(nullptr);
    }

    unhookGraphics();

    overlay_->quit();
    overlay_.reset();

    wait_.set();

    LOGGER("n_overlay") << "@trace hook thread exit... ";
}

void HookApp::tryHookGraphics()
{
    if (session::graphicsWindow())
    {
        return;
    }

    async([this]() {
        hookGraphics();
    });
}

void HookApp::tryHookInput()
{
    if (session::inputHooked())
    {
        return;
    }

    async([this]() {
        hookInput();
    });
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

void HookApp::hookInput()
{
    if (session::inputHooked())
        return;
}

bool HookApp::hookD3d9()
{
    if (!session::d3d9Hook())
    {
        auto d3d9Hook = std::make_unique<D3d9Hook>();

        if (d3d9Hook->hook())
        {
            session::saveD3d9Hook(std::move(d3d9Hook));
        }
    }

    return session::d3d9Hooked();
}

bool HookApp::hookDXGI()
{
    if (!session::dxgiHooked())
    {
        auto dxgiHook = std::make_unique<DXGIHook>();

        if (dxgiHook->hook())
        {
            session::saveDxgiHook(std::move(dxgiHook));
        }
    }

    return session::dxgiHooked();
}

