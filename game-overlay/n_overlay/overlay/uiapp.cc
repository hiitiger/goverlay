#include "stable.h"
#include "session.h"
#include "overlay.h"
#include "uiapp.h"
#include "hookapp.h"

UiApp::UiApp()
{
}

UiApp::~UiApp()
{
}

void UiApp::trySetupGraphicsWindow(HWND window)
{
    WCHAR title[256] = {};
    GetWindowTextW(window, title, 256);
    LOGGER("n_overlay") << "window: " << window << ", title:" << title;

    if (session::injectWindow() && window != session::injectWindow())
    {
        return;
    }

    if (session::graphicsWindow())
    {
        if (window != session::graphicsWindow())
        {
            unhookWindow();
            clearWindowState();
            session::setGraphicsWindow(nullptr);
            session::setWindowThreadId(0);

            setup(window);
            HookApp::instance()->overlayConnector()->sendGraphicsWindowSetupInfo();
        }
    }
    else
    {
        setup(window);
        HookApp::instance()->overlayConnector()->sendGraphicsWindowSetupInfo();
    }
}

void UiApp::setup(HWND window)
{
    DWORD threadId = ::GetWindowThreadProcessId(window, nullptr);

    session::setGraphicsWindow(window);
    session::setWindowThreadId(threadId);

    getWindowState(window);

    hookWindow(window);
}

void UiApp::async(const std::function<void()>& task)
{
    DAssert(session::graphicsWindow());

    std::lock_guard<std::mutex> lock(taskLock_);
    tasks_.push_back(task);
}

void UiApp::startInputBlock()
{
    CHECK_THREAD(Threads::Window);

    HookApp::instance()->overlayConnector()->sendInputBlocked();
}

void UiApp::stopInputBlock()
{
    CHECK_THREAD(Threads::Window);

    HookApp::instance()->overlayConnector()->sendInputUnBlocked();
}

void UiApp::hookWindow(HWND window)
{

}

void UiApp::unhookWindow()
{

}

void UiApp::getWindowState(HWND window)
{
    windowFocus_ = GetForegroundWindow() == window;

    GetClientRect(window, &windowClientRect_);
}

void UiApp::clearWindowState()
{
    windowClientRect_ = {0};
    windowFocus_ = 0;
}
