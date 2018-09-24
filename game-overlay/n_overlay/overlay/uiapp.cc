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

void UiApp::toggleInputIntercept()
{
    if (isIntercepting_)
    {
        stopInputIntercept();
    }
    else
    {
        startInputIntercept();
    }
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
    if (nCode >= 0)
    {
        if (!session::overlayEnabled())
        {
            stopInputIntercept();
        }
        if (session::graphicsActive())
        {
            MSG* pMsg = (MSG*)lParam;
            if (pMsg->hwnd == graphicsWindow_ && wParam == PM_REMOVE)
            {
                if (pMsg->message == WM_KEYDOWN || pMsg->message == WM_SYSKEYDOWN)
                {
                    if (checkHotkey())
                    {
                        return 0;
                    }
                }

                if (!isIntercepting_)
                {
                    return CallNextHookEx(msgHook_, nCode, wParam, lParam);
                }

                if (pMsg->message >= WM_MOUSEFIRST && pMsg->message <= WM_MOUSELAST)
                {
                    POINTS pt = MAKEPOINTS(pMsg->lParam);

                    if (overlay_game::pointInRect(pt, windowClientRect_))
                    {
                        if (!HookApp::instance()->overlayConnector()->processMouseMessage(pMsg->message, pMsg->wParam, pMsg->lParam))
                        {
                            if (pMsg->message == WM_LBUTTONUP
                                || pMsg->message == WM_MBUTTONUP)
                            {
                                async([this]() { this->stopInputIntercept(); });
                            }
                        }
                        pMsg->message = WM_NULL;
                        return 0;
                    }
                }

                if ((pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST)
                    || (pMsg->message >= WM_SYSKEYDOWN && pMsg->message <= WM_SYSDEADCHAR))
                {
                    HookApp::instance()->overlayConnector()->processkeyboardMessage(pMsg->message, pMsg->wParam, pMsg->lParam);
                    if (pMsg->message == WM_KEYDOWN)
                    {
                        TranslateMessage(pMsg);
                    }
                    pMsg->message = WM_NULL;
                    return 0;
                }
            }
        }
    }
    return CallNextHookEx(msgHook_, nCode, wParam, lParam);
}

LRESULT UiApp::hookCallWndProc(_In_ int nCode, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
    if (nCode >= 0)
    {
        CWPSTRUCT* cwp = (CWPSTRUCT*)lParam;

        if (cwp->hwnd == graphicsWindow_)
        {
            if (cwp->message == WM_DESTROY)
            {
                LOGGER("n_overlay") << L"WM_DESTROY, " << graphicsWindow_;

                HookApp::instance()->overlayConnector()->sendGraphicsWindowDestroy(graphicsWindow_);

                unhookWindow();
                graphicsWindow_ = nullptr;

                HookApp::instance()->quit();
            }
            else if (cwp->message == WM_SIZE)
            {
                GetClientRect(graphicsWindow_, &windowClientRect_);
                HookApp::instance()->overlayConnector()->sendGraphicsWindowResizeEvent(graphicsWindow_, windowClientRect_.right - windowClientRect_.left, windowClientRect_.bottom - windowClientRect_.top);
            }

            else if (cwp->message == WM_KILLFOCUS)
            {
                windowFocus_ = false;
                HookApp::instance()->overlayConnector()->sendGraphicsWindowFocusEvent(graphicsWindow_, windowFocus_);
            }
            else if (cwp->message == WM_SETFOCUS)
            {
                windowFocus_ = true;
                HookApp::instance()->overlayConnector()->sendGraphicsWindowFocusEvent(graphicsWindow_, windowFocus_);
            }
            else if (cwp->message == WM_SETCURSOR && LOWORD(cwp->lParam) == HTCLIENT)
            {
                if (isIntercepting_ )
                {
                    if (HookApp::instance()->overlayConnector()->processSetCursor())
                    {
                        return 0;
                    }
                }
            }
            else if (cwp->message == WM_NCHITTEST)
            {
                if (isIntercepting_)
                {
                    HookApp::instance()->overlayConnector()->processNCHITTEST(cwp->message, cwp->wParam, cwp->lParam);
                }
            }
        }
    }

    return CallNextHookEx(wndProcHook_, nCode, wParam, lParam);
}

LRESULT UiApp::hookCallWndRetProc(_In_ int nCode, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
    if (nCode >= 0)
    {
        CWPRETSTRUCT * cwp = (CWPRETSTRUCT *)lParam;

        if (cwp->hwnd == graphicsWindow_)
        {
            if (cwp->message == WM_SETCURSOR && LOWORD(cwp->lParam) == HTCLIENT)
            {
                if (isIntercepting_)
                {
                    if (HookApp::instance()->overlayConnector()->processSetCursor())
                    {
                        return 0;
                    }
                }
            }
        }
    }
    return CallNextHookEx(wndRetProcHook_, nCode, wParam, lParam);
}

bool UiApp::checkHotkey()
{
    if (Windows::OrginalApi::GetAsyncKeyState(VK_F1))
    {
        toggleInputIntercept();
        return true;
    }

    return false;
}

