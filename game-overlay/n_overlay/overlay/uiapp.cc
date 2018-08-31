#include "stable.h"
#include "session.h"
#include "overlay.h"
#include "uiapp.h"
#include "hookapp.h"
#include "hook/inputhook.h"


UiApp::UiApp()
{
}

UiApp::~UiApp()
{
}

bool UiApp::trySetupGraphicsWindow(HWND window)
{
    WCHAR title[256] = {};
    GetWindowTextW(window, title, 256);
    LOGGER("n_overlay") << "window: " << window << ", title:" << title;

    if (session::graphicsWindow() && window != session::graphicsWindow())
    {
        return false;
    }

    if (graphicsWindow_ == window)
    {
        return true;
    }

    return setup(window);
}

bool UiApp::setup(HWND window)
{
    std::lock_guard<std::mutex> lock(uilock_);

    bool focused = GetForegroundWindow() == window;
    RECT rect = { 0 };
    GetClientRect(window, &rect);

    if (hookWindow(window))
    {
        graphicsWindow_ = window;

        windowFocus_ = focused;
        windowClientRect_ = rect;

        HookApp::instance()->overlayConnector()->sendGraphicsWindowSetupInfo(window, rect.right - rect.left, rect.bottom - rect.top, focused, true);

        return true;
    }
    else
    {
        HookApp::instance()->overlayConnector()->sendGraphicsWindowSetupInfo(window, rect.right - rect.left, rect.bottom - rect.top, focused, false);

        unhookWindow();
        return false;
    }
}

bool UiApp::windowSetted() const
{
    return !!graphicsWindow_.load();
}

void UiApp::async(const std::function<void()>& task)
{
    DAssert(session::graphicsWindow());

    std::lock_guard<std::mutex> lock(taskLock_);
    tasks_.push_back(task);
}

void UiApp::startInputIntercept()
{
    CHECK_THREAD(Threads::Window);

    if (!isIntercepting_)
    {
        isIntercepting_ = true;
        session::inputHook()->saveInputState();
        HookApp::instance()->overlayConnector()->sendInputIntercept();
    }
}

void UiApp::stopInputIntercept()
{
    CHECK_THREAD(Threads::Window);

    if (isIntercepting_)
    {
        isIntercepting_ = false;
        session::inputHook()->restoreInputState();
        HookApp::instance()->overlayConnector()->sendInputStopIntercept();
    }
}

bool UiApp::isInterceptingInput()
{
    return isIntercepting_;
}

bool UiApp::hookWindow(HWND window)
{
    DWORD threadId = ::GetWindowThreadProcessId(window, nullptr);

    msgHook_ = SetWindowsHookExW(WH_GETMESSAGE, GetMsgProc, NULL, threadId);
    wndProcHook_ = SetWindowsHookExW(WH_CALLWNDPROC, CallWndProc, NULL, threadId);
    wndRetProcHook_ = SetWindowsHookExW(WH_CALLWNDPROCRET, CallWndRetProc, NULL, threadId);

    return msgHook_ != nullptr && wndProcHook_ != nullptr && wndRetProcHook_ != nullptr;
}

void UiApp::unhookWindow()
{
    if (msgHook_)
    {
        UnhookWindowsHookEx(msgHook_);
        msgHook_ = nullptr;
    }
    if (wndProcHook_)
    {
        UnhookWindowsHookEx(wndProcHook_);
        wndProcHook_ = nullptr;
    }
    if (wndRetProcHook_)
    {
        UnhookWindowsHookEx(wndRetProcHook_);
        wndRetProcHook_ = nullptr;
    }
}

void UiApp::updateWindowState(HWND window)
{
    windowFocus_ = GetForegroundWindow() == window;

    GetClientRect(window, &windowClientRect_);
}

void UiApp::clearWindowState()
{
    windowClientRect_ = {0};
    windowFocus_ = 0;
}


LRESULT CALLBACK UiApp::GetMsgProc(_In_ int nCode, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
    return HookApp::instance()->uiapp()->hookGetMsgProc(nCode, wParam, lParam);
}

LRESULT CALLBACK UiApp::CallWndProc(_In_ int nCode, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
    return HookApp::instance()->uiapp()->hookCallWndProc(nCode, wParam, lParam);
}

LRESULT CALLBACK UiApp::CallWndRetProc(_In_ int nCode, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
    return HookApp::instance()->uiapp()->hookCallWndRetProc(nCode, wParam, lParam);
}

LRESULT UiApp::hookGetMsgProc(_In_ int nCode, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
    return CallNextHookEx(msgHook_, nCode, wParam, lParam);
}

LRESULT UiApp::hookCallWndProc(_In_ int nCode, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
    return CallNextHookEx(wndProcHook_, nCode, wParam, lParam);
}

LRESULT UiApp::hookCallWndRetProc(_In_ int nCode, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
    return CallNextHookEx(wndRetProcHook_, nCode, wParam, lParam);
}

void UiApp::checkHotkey()
{
    //
}

