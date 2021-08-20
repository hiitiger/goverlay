#include "stable.h"
#include "session.h"
#include "overlay.h"
#include "uiapp.h"
#include "hookapp.h"
#include "hook/inputhook.h"
#include "hotkey/hotkeycheck.h"

#if ALLOW_ASSOC_SYSIME
#pragma comment(lib, "imm32.lib")
#endif


#define  OVERLAY_MAGIC 0x908988
#define  OVERLAY_TASK 0x908987

const bool g_use_wndproc_hook = true;

UiApp::UiApp()
{
    overlayMagicMsg_ = RegisterWindowMessageW(L"n_overlay_0x010101");
    if (overlayMagicMsg_ == 0)
    {
        overlayMagicMsg_ = WM_USER + 0x88;
    }

    HookApp::instance()->overlayConnector()->remoteConnectEvent().add([this](){
        if (this->graphicsWindow_)
            HookApp::instance()->overlayConnector()->sendGraphicsWindowSetupInfo(graphicsWindow_, windowClientRect_.right - windowClientRect_.left, windowClientRect_.bottom - windowClientRect_.top, windowFocus_, true);
    }, this);
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

    if (graphicsWindow_)
    {
        unhookWindow();
        graphicsWindow_ = nullptr;
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

        async([this]() {
            if (this->graphicsWindow_)
            {
                if (GetForegroundWindow() == this->graphicsWindow_)
                {
                    HookApp::instance()->overlayConnector()->translateWindowsToGameClient();
                }
            }
        });

        return true;
    }
    else
    {
        HookApp::instance()->overlayConnector()->sendGraphicsWindowSetupInfo(window, rect.right - rect.left, rect.bottom - rect.top, focused, false);

        unhookWindow();
        return false;
    }
}

HWND UiApp::window() const
{
    return graphicsWindow_.load();
}

bool UiApp::windowSetted() const
{
    return !!graphicsWindow_.load();
}

bool UiApp::windowFocused() const
{
    return windowFocus_;
}

void UiApp::async(const std::function<void()>& task)
{
    DAssert(session::graphicsWindow());

    std::lock_guard<std::mutex> lock(taskLock_);
    tasks_.push_back(task);

    PostMessage(graphicsWindow_, overlayMagicMsg_, OVERLAY_MAGIC, OVERLAY_TASK);
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

    if (session::overlayEnabled())
    {
        if (!isIntercepting_)
        {
            isIntercepting_ = true;

            if (session::isWindowed())
            {
#if ALLOW_ASSOC_SYSIME
                if (!IMC_)
                {
                    IMC_ = ImmCreateContext();
                }
                originalIMC_ = ImmAssociateContext(graphicsWindow_, IMC_);
#endif
            }

            session::inputHook()->saveInputState();
            HookApp::instance()->overlayConnector()->sendInputIntercept();

            POINT pt{};
            Windows::OrginalApi::GetCursorPos(&pt);
            LPARAM lParam = 0;
            lParam = pt.x + (pt.y << 16);
            HookApp::instance()->overlayConnector()->processNCHITTEST(WM_NCHITTEST, 0, lParam);
            HookApp::instance()->overlayConnector()->processSetCursor();


#if AUTO_INPUT_INTERCEPT
            stopAutoIntercept();
#endif
        }
    }
}

void UiApp::stopInputIntercept()
{
    CHECK_THREAD(Threads::Window);

    if (session::overlayEnabled())
    {
        if (isIntercepting_)
        {
            isIntercepting_ = false;

#if ALLOW_ASSOC_SYSIME
            if (!originalIMC_)
            {
                ImmAssociateContext(graphicsWindow_, nullptr);
            }
            else
            {
                ImmAssociateContext(graphicsWindow_, originalIMC_);
                originalIMC_ = nullptr;
            }
            if (IMC_)
            {
                ImmReleaseContext(graphicsWindow_, IMC_);
                IMC_ = nullptr;
            }
#endif
            session::inputHook()->restoreInputState();
            HookApp::instance()->overlayConnector()->sendInputStopIntercept();
        }
    }
}

#if AUTO_INPUT_INTERCEPT
void UiApp::startAutoIntercept()
{
    if (session::overlayEnabled())
    {
        if (!isInterceptingMouseAuto_)
        {
            isInterceptingMouseAuto_ = true;
        }
    }
}
#endif

#if AUTO_INPUT_INTERCEPT
void UiApp::stopAutoIntercept()
{
    if (session::overlayEnabled())
    {
        if (isInterceptingMouseAuto_)
        {
            isInterceptingMouseAuto_ = false;
        }
    }
}
#endif

bool UiApp::shouldBlockOrginalMouseInput()
{
#if AUTO_INPUT_INTERCEPT
    //return isInterceptingMouseAuto_ || isIntercepting_;
    return isIntercepting_;
#else
    return isIntercepting_;
#endif
}

bool UiApp::shouldBlockOrginalKeyInput()
{
#if AUTO_INPUT_INTERCEPT
    return (isInterceptingMouseAuto_ && HookApp::instance()->overlayConnector()->focusWindowId() != 0) || isIntercepting_;
#else
    return isIntercepting_;
#endif
}

bool UiApp::shouldBlockOrginalCursorViz()
{
    return isIntercepting_;
}

bool UiApp::isInterceptingInput()
{
    return isIntercepting_;
}

#if AUTO_INPUT_INTERCEPT
bool UiApp::isInterceptingMouseAuto()
{
    return isInterceptingMouseAuto_;
}
#endif

bool UiApp::hookWindow(HWND window)
{
    if (g_use_wndproc_hook)
    {
        oldWndProc_ = (WNDPROC)GetWindowLongPtr(window, GWLP_WNDPROC);
        SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)WindowProc);
        return oldWndProc_ != nullptr;
    }
    else
    {
        DWORD threadId = ::GetWindowThreadProcessId(window, nullptr);

        msgHook_ = SetWindowsHookExW(WH_GETMESSAGE, GetMsgProc, NULL, threadId);
        wndProcHook_ = SetWindowsHookExW(WH_CALLWNDPROC, CallWndProc, NULL, threadId);
        wndRetProcHook_ = SetWindowsHookExW(WH_CALLWNDPROCRET, CallWndRetProc, NULL, threadId);

        return msgHook_ != nullptr && wndProcHook_ != nullptr && wndRetProcHook_ != nullptr;
    }
}

void UiApp::unhookWindow()
{
    if (g_use_wndproc_hook)
    {
        if (oldWndProc_)
        {
            SetWindowLongPtr(graphicsWindow_, GWLP_WNDPROC, (LONG_PTR)oldWndProc_);
        }
    }
    else
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

std::uint32_t UiApp::gameWidth() const
{
    return windowClientRect_.right - windowClientRect_.left;
}

std::uint32_t UiApp::gameHeight() const
{
    return windowClientRect_.bottom - windowClientRect_.top;
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

LRESULT WINAPI UiApp::WindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    return HookApp::instance()->uiapp()->hookWindowProc(hWnd, Msg, wParam, lParam);
}


LRESULT UiApp::hookWindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (!session::overlayEnabled())
    {
        stopInputIntercept();
    }
    if (session::graphicsActive())
    {
        if (Msg == WM_KEYDOWN || Msg == WM_SYSKEYDOWN
            || Msg == WM_KEYUP || Msg == WM_SYSKEYUP)
        {
            if (checkHotkey())
            {
                return 0;
            }
        }

        if (Msg == overlayMagicMsg_ && wParam == OVERLAY_MAGIC)
        {
            if (lParam == OVERLAY_TASK)
            {
                _runTask();
            }
        }
    }

    if (Msg == WM_DESTROY)
    {
        LOGGER("n_overlay") << L"WM_DESTROY, " << graphicsWindow_;

        HookApp::instance()->overlayConnector()->sendGraphicsWindowDestroy(graphicsWindow_);

        unhookWindow();
        graphicsWindow_ = nullptr;

        HookApp::instance()->quit();
    }
    else if (Msg == WM_SIZE)
    {
        GetClientRect(graphicsWindow_, &windowClientRect_);
        HookApp::instance()->overlayConnector()->sendGraphicsWindowResizeEvent(graphicsWindow_, windowClientRect_.right - windowClientRect_.left, windowClientRect_.bottom - windowClientRect_.top);
    }

    else if (Msg == WM_KILLFOCUS)
    {
        windowFocus_ = false;
        HookApp::instance()->overlayConnector()->sendGraphicsWindowFocusEvent(graphicsWindow_, windowFocus_);
#if AUTO_INPUT_INTERCEPT
        stopAutoIntercept();
#endif
        stopInputIntercept();

    }
    else if (Msg == WM_SETFOCUS)
    {
        windowFocus_ = true;
        HookApp::instance()->overlayConnector()->sendGraphicsWindowFocusEvent(graphicsWindow_, windowFocus_);
    }
    else if (Msg == WM_SETCURSOR && LOWORD(lParam) == HTCLIENT)
    {
        if (_setCusror())
        {
            return 0;
        }
    }
    else if (Msg == WM_NCHITTEST)
    {
        if (isIntercepting_)
        {
            HookApp::instance()->overlayConnector()->processNCHITTEST(Msg, wParam, lParam);
        }
#if AUTO_INPUT_INTERCEPT
        else
        {
            HookApp::instance()->overlayConnector()->processNCHITTEST(Msg, wParam, lParam, false) ? startAutoIntercept() : stopAutoIntercept();
        }
#endif
    }

    if (session::graphicsActive())
    {
        if (!isIntercepting_)
        {

#if AUTO_INPUT_INTERCEPT
            if (!isInterceptingMouseAuto_ && !HookApp::instance()->overlayConnector()->isMousePressingOnOverlayWindow())
            {
                return CallWindowProc(oldWndProc_, hWnd, Msg, wParam, lParam);
            }

#else
            return CallWindowProc(oldWndProc_, hWnd, Msg, wParam, lParam);
#endif
        }

        if (Msg >= WM_MOUSEFIRST && Msg <= WM_MOUSELAST)
        {
            POINTS pt = MAKEPOINTS(lParam);
#if AUTO_INPUT_INTERCEPT
            if (!HookApp::instance()->overlayConnector()->processMouseMessage(Msg, wParam, lParam, isIntercepting_))
#else
            if (!HookApp::instance()->overlayConnector()->processMouseMessage(Msg, wParam, lParam))
#endif // AUTO_INPUT_INTERCEPT

            {
                if (Msg == WM_LBUTTONUP
                    || Msg == WM_MBUTTONUP)
                {
                    async([this]() { this->stopInputIntercept(); });
                }
            }
            return 0;
        }

        if (Msg == WM_INPUT)
        {
            return 0;
        }

        if ((Msg >= WM_KEYFIRST && Msg <= WM_KEYLAST)
            || (Msg >= WM_SYSKEYDOWN && Msg <= WM_SYSDEADCHAR))
        {
            bool inputHandled = HookApp::instance()->overlayConnector()->processkeyboardMessage(Msg, wParam, lParam);
            if (inputHandled)
            {
                /*if (Msg == WM_KEYDOWN)
                {
                    if (!HookApp::instance()->overlayConnector()->directMessageInput())
                    {
                        TranslateMessage(pMsg);
                    }
                }*/
            }
            return 0;
        }
    }

    return CallWindowProc(oldWndProc_, hWnd, Msg, wParam, lParam);
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
                if (pMsg->message == WM_KEYDOWN || pMsg->message == WM_SYSKEYDOWN
                    || pMsg->message == WM_KEYUP || pMsg->message == WM_SYSKEYUP)
                {
                    if (checkHotkey())
                    {
                        return 0;
                    }
                }

                if (pMsg->message == overlayMagicMsg_ && pMsg->wParam == OVERLAY_MAGIC)
                {
                    if (pMsg->lParam == OVERLAY_TASK)
                    {
                        _runTask();
                    }
                }

                if (!isIntercepting_)
                {
                    
#if AUTO_INPUT_INTERCEPT
                    if (!isInterceptingMouseAuto_ && !HookApp::instance()->overlayConnector()->isMousePressingOnOverlayWindow())
                    {
                        return CallNextHookEx(msgHook_, nCode, wParam, lParam);
                    }
                    
#else
                    return CallNextHookEx(msgHook_, nCode, wParam, lParam);
#endif
                }

                if (pMsg->message >= WM_MOUSEFIRST && pMsg->message <= WM_MOUSELAST)
                {
                    POINTS pt = MAKEPOINTS(pMsg->lParam);

                    if (overlay_game::pointInRect(pt, windowClientRect_))
                    {
#if AUTO_INPUT_INTERCEPT
                        if (!HookApp::instance()->overlayConnector()->processMouseMessage(pMsg->message, pMsg->wParam, pMsg->lParam, isIntercepting_))
#else
                        if (!HookApp::instance()->overlayConnector()->processMouseMessage(pMsg->message, pMsg->wParam, pMsg->lParam))
#endif // AUTO_INPUT_INTERCEPT

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
                    bool inputHandled = HookApp::instance()->overlayConnector()->processkeyboardMessage(pMsg->message, pMsg->wParam, pMsg->lParam);
                    if (inputHandled)
                    {
                        if (pMsg->message == WM_KEYDOWN)
                        {
                            if (!HookApp::instance()->overlayConnector()->directMessageInput())
                            {
                                TranslateMessage(pMsg);
                            }
                        }
                        pMsg->message = WM_NULL;
                    }
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
#if AUTO_INPUT_INTERCEPT
                stopAutoIntercept();
#endif
                stopInputIntercept();

            }
            else if (cwp->message == WM_SETFOCUS)
            {
                windowFocus_ = true;
                HookApp::instance()->overlayConnector()->sendGraphicsWindowFocusEvent(graphicsWindow_, windowFocus_);
            }
            else if (cwp->message == WM_SETCURSOR && LOWORD(cwp->lParam) == HTCLIENT)
            {
                if (_setCusror())
                {
                    return 0;
                }
            }
            else if (cwp->message == WM_NCHITTEST)
            {
                if (isIntercepting_)
                {
                    HookApp::instance()->overlayConnector()->processNCHITTEST(cwp->message, cwp->wParam, cwp->lParam);
                }
#if AUTO_INPUT_INTERCEPT
                else
                {
                    HookApp::instance()->overlayConnector()->processNCHITTEST(cwp->message, cwp->wParam, cwp->lParam, false) ? startAutoIntercept() : stopAutoIntercept();
                }
#endif
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
                if (_setCusror())
                {
                    return 0;
                }
            }
        }
    }
    return CallNextHookEx(wndRetProcHook_, nCode, wParam, lParam);
}

bool UiApp::checkHotkey()
{
#if defined(_DEBUG) || defined(DEBUG)
    if (Windows::OrginalApi::GetAsyncKeyState(VK_F1))
    {
        toggleInputIntercept();
        return true;
    }
#endif

#ifndef HOTKEY_THREADED
    HotkeyCheck::instance()->checkHotkeys();
#endif // HOTKEY_THREADED

    return false;
}

void UiApp::_runTask()
{
    std::deque<std::function<void()>> tasks;
    {
        std::lock_guard<std::mutex> lock(taskLock_);
        tasks.swap(tasks_);
    }

    for (auto& task : tasks)
    {
        task();
    }
}

bool UiApp::_setCusror()
{
    if (isIntercepting_)
    {
        if (HookApp::instance()->overlayConnector()->processSetCursor())
        {
            return true;
        }
    }

#if AUTO_INPUT_INTERCEPT
    else if (isInterceptingMouseAuto_)
    {
        if (HookApp::instance()->overlayConnector()->processSetCursor())
        {
            return true;
        }
    }
#endif

    return false;
}

