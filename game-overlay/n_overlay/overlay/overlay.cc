#include "stable.h"
#include "overlay.h"
#include "hookapp.h"
#include "hook/inputhook.h"
#include <boost/range/adaptor/reversed.hpp>
const char k_overlayIpcName[] = "n_overlay_1a1y2o8l0b";


static auto _syncDragResizeLeft = [&](auto& window, std::int32_t xdiff, std::int32_t /*ydiff*/) {
    auto curWidth = window->rect.width;
    auto curRight = window->rect.width + window->rect.x;

    curWidth -= xdiff;
    if (curWidth < (int)window->minWidth)
        curWidth = window->minWidth;
    else if (curWidth > (int)window->maxWidth)
        curWidth = window->maxWidth;
    window->rect.x = curRight - curWidth;
    window->rect.width = curWidth;
};

static auto _syncDragResizeRight = [&](auto& window, std::int32_t xdiff, std::int32_t /*ydiff*/) {
    auto curWidth = window->rect.width;

    curWidth += xdiff;
    if (curWidth < (int)window->minWidth)
        curWidth = window->minWidth;
    else if (curWidth > (int)window->maxHeight)
        curWidth = window->maxHeight;
    window->rect.width = curWidth;
};

static auto _syncDragResizeTop = [&](auto& window, std::int32_t /*xdiff*/, std::int32_t ydiff) {
    auto curHeight = window->rect.height;
    auto curBottom = window->rect.height + window->rect.y;
    curHeight -= ydiff;
    if (curHeight < (int)window->minHeight)
        curHeight = window->minHeight;
    else if (curHeight > (int)window->maxHeight)
        curHeight = window->maxHeight;
    window->rect.y = curBottom - curHeight;
    window->rect.height = curHeight;
};

static auto _syncDragResizeBottom = [&](auto& window, std::int32_t /*xdiff*/, std::int32_t ydiff) {
    auto curHeight = window->rect.height;
    curHeight += ydiff;
    if (curHeight < (int)window->minHeight)
        curHeight = window->minHeight;
    else if (curHeight > (int)window->maxHeight)
        curHeight = window->maxHeight;
    window->rect.height = curHeight;
};


OverlayConnector::OverlayConnector()
{
    arrowCursor_ = (HCURSOR)::LoadImageW(NULL, IDC_ARROW, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
    ibeamCursor_ = (HCURSOR)::LoadImageW(NULL, IDC_IBEAM, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
    handCusor_ = (HCURSOR)::LoadImageW(NULL, IDC_HAND, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
    crossCusor_ = (HCURSOR)::LoadImageW(NULL, IDC_CROSS, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
    waitCusor_ = (HCURSOR)::LoadImageW(NULL, IDC_WAIT, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
    helpCusor_ = (HCURSOR)::LoadImageW(NULL, IDC_HELP, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
    sizeAllCusor_ = (HCURSOR)::LoadImageW(NULL, IDC_SIZEALL, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
    sizeNWSECusor_ = (HCURSOR)::LoadImageW(NULL, IDC_SIZENWSE, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
    sizeNESWCusor_ = (HCURSOR)::LoadImageW(NULL, IDC_SIZENESW, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
    sizeNSCusor_ = (HCURSOR)::LoadImageW(NULL, IDC_SIZENS, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
    sizeWECusor_ = (HCURSOR)::LoadImageW(NULL, IDC_SIZEWE, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
}

OverlayConnector::~OverlayConnector()
{
}

void OverlayConnector::start()
{
    __trace__;

    CHECK_THREAD(Threads::HookApp);

    std::string ipcName = k_overlayIpcName;
    ipcName.append("-");
    ipcName.append(win_utils::toLocal8Bit(HookApp::instance()->procName()));
    ipcName.append("-");
    ipcName.append(std::to_string(::GetCurrentProcessId()));
    getIpcCenter()->init(ipcName);

    std::string mainIpcName = k_overlayIpcName;
    ipcLink_ = getIpcCenter()->getLink(mainIpcName);
    ipcLink_->addClient(this);
    getIpcCenter()->connectToHost(ipcLink_, "", "", false);
}

void OverlayConnector::quit()
{
    __trace__;

    CHECK_THREAD(Threads::HookApp);

    if (ipcLink_)
    {
        _sendGameExit();
        getIpcCenter()->closeLink(ipcLink_);
        ipcLink_ = nullptr;
    }
    getIpcCenter()->uninit();
}

void OverlayConnector::sendInputHookInfo(bool hooked)
{
    CHECK_THREAD(Threads::HookApp);

    _sendInputHookInfo(hooked);
}

void OverlayConnector::sendGraphicsHookInfo(const overlay_game::D3d9HookInfo &info)
{
    CHECK_THREAD(Threads::HookApp);

    HookApp::instance()->async([this, info]() {
        _sendGraphicsHookInfo(info);
    });
}

void OverlayConnector::sendGraphicsHookInfo(const overlay_game::DxgiHookInfo &info)
{
    CHECK_THREAD(Threads::HookApp);

    HookApp::instance()->async([this, info]() {
        _sendGraphicsHookInfo(info);
    });
}

void OverlayConnector::sendGraphicsWindowSetupInfo(HWND window, int width, int height, bool focus, bool hooked)
{
    std::cout << __FUNCTION__ << "hooked: " << hooked << std::endl;
    CHECK_THREAD(Threads::HookApp);

    HookApp::instance()->async([this, window, width, height, focus, hooked]() {
        _sendGraphicsWindowSetupInfo(window, width, height, focus, hooked);
    });
}

void OverlayConnector::sendGraphicsWindowResizeEvent(HWND window, int width, int height)
{
    CHECK_THREAD(Threads::Window);
    HookApp::instance()->async([this, window, width, height]() {
        _sendGraphicsWindowResizeEvent(window, width, height);
    });
}

void OverlayConnector::sendGraphicsWindowFocusEvent(HWND window, bool focus)
{
    CHECK_THREAD(Threads::Window);
    HookApp::instance()->async([this, window, focus]() {
        _sendGraphicsWindowFocusEvent(window, focus);
    });

    if (!focus)
    {
        clearMouseDrag();
    }
}

void OverlayConnector::sendGraphicsWindowDestroy(HWND window)
{
    CHECK_THREAD(Threads::Window);
    HookApp::instance()->async([this, window]() {
        _sendGraphicsWindowDestroy(window);
    });
}

void OverlayConnector::sendGraphicsFps(std::uint32_t fps)
{
    CHECK_THREAD(Threads::Graphics);
    HookApp::instance()->async([this, fps]() {
        _sendGraphicsFps(fps);
    });
}

void OverlayConnector::sendInputIntercept()
{
    CHECK_THREAD(Threads::Window);
    HookApp::instance()->async([this]() {
        _sendInputIntercept(true);
    });
}

void OverlayConnector::sendInputStopIntercept()
{
    CHECK_THREAD(Threads::Window);
    HookApp::instance()->async([this]() {
        _sendInputIntercept(false);
    });
}

const std::vector<std::shared_ptr<overlay::Window>>& OverlayConnector::windows()
{
    return windows_;
}

void OverlayConnector::lockShareMem()
{
    shareMemoryLock_.lock();
}

void OverlayConnector::unlockShareMem()
{
    shareMemoryLock_.unlock();
}

void OverlayConnector::lockWindows()
{
    windowsLock_.lock();
}

void OverlayConnector::unlockWindows()
{
    windowsLock_.unlock();
}

bool OverlayConnector::processNCHITTEST(UINT /*message*/, WPARAM /*wParam*/, LPARAM lParam)
{
    if (dragMoveWindowId_ != 0)
    {
        return false;
    }

    POINTS screenPoint = MAKEPOINTS(lParam);
    POINT mousePointInGameClient = { screenPoint.x, screenPoint.y };
    ScreenToClient(session::graphicsWindow(), &mousePointInGameClient);

    std::lock_guard<std::mutex> lock(windowsLock_);

    for (auto& window :boost::adaptors::reverse(windows_))
    {
        if (overlay_game::pointInRect(mousePointInGameClient, window->rect))
        {
            POINT mousePointinWindowClient = { mousePointInGameClient.x, mousePointInGameClient.y };
            mousePointinWindowClient.x -= window->rect.x;
            mousePointinWindowClient.y -= window->rect.y;

            hitTest_ = overlay_game::hitTest(mousePointinWindowClient, window->rect, window->resizable, window->caption.value(), window->dragBorderWidth);
            return false;
        }
    }

    hitTest_ = HTNOWHERE;

    return false;
}

bool OverlayConnector::processMouseMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
    std::lock_guard<std::mutex> lock(windowsLock_);
    POINT mousePointInGameClient{ LOWORD(lParam), HIWORD(lParam) };

    {
        std::lock_guard<std::recursive_mutex> lock(mouseDragLock_);
        //move by caption hittest
        if (dragMoveWindowId_ != 0)
        {
            auto it = std::find_if(windows_.begin(), windows_.end(), [&](const auto& window) {
                return window->windowId == dragMoveWindowId_;
            });
            if (it != windows_.end())
            {
                if (message == WM_MOUSEMOVE)
                {
                    auto& window = *it;

                    POINT mousePointinWindowClient = { mousePointInGameClient.x, mousePointInGameClient.y };
                    mousePointinWindowClient.x -= window->rect.x;
                    mousePointinWindowClient.y -= window->rect.y;

                    int xdiff = mousePointinWindowClient.x - dragMoveLastMousePos_.x;
                    int ydiff = mousePointinWindowClient.y - dragMoveLastMousePos_.y;

                    if (dragMoveMode_ == HTCAPTION)
                    {
                        window->rect.x += xdiff;
                        window->rect.y += ydiff;

                        dragMoveLastMousePos_.x = mousePointInGameClient.x - window->rect.x;
                        dragMoveLastMousePos_.y = mousePointInGameClient.y - window->rect.y;

                        SetWindowPos((HWND)window->nativeHandle, NULL, window->rect.x, window->rect.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

                        this->windowBoundsEvent()(dragMoveWindowId_, window->rect);
                    }
                    else
                    {
                        if (dragMoveMode_ == HTLEFT)
                        {
                            _syncDragResizeLeft(window, xdiff, ydiff);
                        }
                        else if (dragMoveMode_ == HTRIGHT)
                        {
                            _syncDragResizeRight(window, xdiff, ydiff);
                        }
                        else if (dragMoveMode_ == HTTOP)
                        {
                            _syncDragResizeTop(window, xdiff, ydiff);
                        }
                        else if (dragMoveMode_ == HTBOTTOM)
                        {
                            _syncDragResizeBottom(window, xdiff, ydiff);
                        }
                        else if (dragMoveMode_ == HTTOPLEFT)
                        {
                            _syncDragResizeLeft(window, xdiff, ydiff);
                            _syncDragResizeTop(window, xdiff, ydiff);
                        }
                        else if (dragMoveMode_ == HTTOPRIGHT)
                        {
                            _syncDragResizeRight(window, xdiff, ydiff);
                            _syncDragResizeTop(window, xdiff, ydiff);
                        }
                        else if (dragMoveMode_ == HTBOTTOMLEFT)
                        {
                            _syncDragResizeLeft(window, xdiff, ydiff);
                            _syncDragResizeBottom(window, xdiff, ydiff);
                        }
                        else if (dragMoveMode_ == HTBOTTOMRIGHT)
                        {
                            _syncDragResizeRight(window, xdiff, ydiff);
                            _syncDragResizeBottom(window, xdiff, ydiff);
                        }

                        dragMoveLastMousePos_.x = mousePointInGameClient.x - window->rect.x;
                        dragMoveLastMousePos_.y = mousePointInGameClient.y - window->rect.y;

                        SetWindowPos((HWND)window->nativeHandle, NULL, window->rect.x, window->rect.y, window->rect.width, window->rect.height, SWP_NOZORDER | SWP_NOACTIVATE);

                        this->windowBoundsEvent()(dragMoveWindowId_, window->rect);
                    }
                    return true;
                }
                else if (message == WM_LBUTTONUP)
                {
                    clearMouseDrag();
                }
            }
            else
            {
                clearMouseDrag();
            }
            return true;
        }
    }
   
    if (message == WM_MOUSEWHEEL)
    {
        POINT gx = { 0, 0 };
        ClientToScreen(session::graphicsWindow(), &gx);

        mousePointInGameClient.x -= (SHORT)gx.x;
        mousePointInGameClient.y -= (SHORT)gx.y;
    }

    if (mousePressWindowId_)
    {
        auto it = std::find_if(windows_.begin(), windows_.end(), [&](const auto& window) {
            return window->windowId == mousePressWindowId_;
        });

        if (it != windows_.end())
        {
            auto& window = *it;

            POINT mousePointinWindowClient = { mousePointInGameClient.x, mousePointInGameClient.y };
            mousePointinWindowClient.x -= window->rect.x;
            mousePointinWindowClient.y -= window->rect.y;

            DWORD pos = mousePointinWindowClient.x + (mousePointinWindowClient.y << 16);
            lParam = (LPARAM)pos;

            HookApp::instance()->async([this, windowId = window->windowId, message, wParam, lParam]() {
                _sendGameWindowInput(windowId, message, wParam, lParam);
            });

            if (message == WM_LBUTTONUP)
            {
                mousePressWindowId_ = 0;
            }
        }
        else
        {
            mousePressWindowId_ = 0;
        }

        return true;
    }

    for (auto & window :boost::adaptors::reverse(windows_))
    {
        if (overlay_game::pointInRect(mousePointInGameClient, window->rect))
        {
            POINT mousePointinWindowClient = { mousePointInGameClient.x, mousePointInGameClient.y };
            mousePointinWindowClient.x -= window->rect.x;
            mousePointinWindowClient.y -= window->rect.y;

            //even for mousewheel we translate it to local cord

            DWORD pos = mousePointinWindowClient.x + (mousePointinWindowClient.y << 16);
            lParam = (LPARAM)pos;

            if (message == WM_LBUTTONDOWN)
            {
                mousePressWindowId_ = window->windowId;
                focusWindowId_ = window->windowId;

                if (hitTest_ == HTCAPTION)
                {
                    std::lock_guard<std::recursive_mutex> lock(mouseDragLock_);
                    dragMoveWindowId_ = window->windowId;
                    dragMoveWindowHandle_ = window->nativeHandle;
                    dragMoveLastMousePos_.x = mousePointinWindowClient.x;
                    dragMoveLastMousePos_.y = mousePointinWindowClient.y;
                    dragMoveMode_ = HTCAPTION;
                }
                else if(hitTest_ != HTCLIENT)
                {
                    std::lock_guard<std::recursive_mutex> lock(mouseDragLock_);
                    dragMoveWindowId_ = window->windowId;
                    dragMoveWindowHandle_ = window->nativeHandle;
                    dragMoveLastMousePos_.x = mousePointinWindowClient.x;
                    dragMoveLastMousePos_.y = mousePointinWindowClient.y;
                    dragMoveMode_ = hitTest_;
                }
            }
            else if (message == WM_LBUTTONUP)
            {
                mousePressWindowId_ = 0;
            }

            if (dragMoveWindowId_ == 0)
            {
                HookApp::instance()->async([this, windowId = window->windowId, message, wParam, lParam]() {
                    _sendGameWindowInput(windowId, message, wParam, lParam);
                });
            }

            if (focusWindowId_)
            {
                if (windows_.at(windows_.size() -1)->windowId != focusWindowId_)
                {
                    auto it = std::find_if(windows_.begin(), windows_.end(), [&](const auto& w) {
                        return w->windowId == focusWindowId_;
                    });

                    auto focusWindow = *it;
                    windows_.erase(it);
                    windows_.push_back(focusWindow);

                    this->windowFocusEvent()(focusWindowId_);
                }
            }

            return true;
        }
    }

    // notify mouse is not accepted
    HookApp::instance()->async([this, windowId = 0, message, wParam, lParam]() {
        _sendGameWindowInput(windowId, message, wParam, lParam);
    });

    if (message == WM_LBUTTONDOWN)
    {
        focusWindowId_ = 0;
    }

    return false;
}

bool OverlayConnector::processkeyboardMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
    if (focusWindowId_ != 0)
    {
        HookApp::instance()->async([this, windowId = focusWindowId_.load(), message, wParam, lParam]() {
            _sendGameWindowInput(windowId, message, wParam, lParam);
        });
    }
    return true;
}

bool OverlayConnector::processSetCursor()
{
    if (hitTest_ == HTCAPTION)
    {
        Windows::OrginalApi::SetCursor(arrowCursor_);
        return true;
    }

    if (hitTest_ == HTLEFT || hitTest_ == HTRIGHT)
    {
        Windows::OrginalApi::SetCursor(sizeWECusor_);
        return true;
    }

    if (hitTest_ == HTTOP || hitTest_ == HTBOTTOM)
    {
        Windows::OrginalApi::SetCursor(sizeNSCusor_);
        return true;
    }

    if (hitTest_ == HTTOPLEFT || hitTest_ == HTBOTTOMRIGHT)
    {
        Windows::OrginalApi::SetCursor(sizeNWSECusor_);
        return true;
    }

    if (hitTest_ == HTTOPRIGHT || hitTest_ == HTBOTTOMLEFT)
    {
        Windows::OrginalApi::SetCursor(sizeNESWCusor_);
        return true;
    }

    if(cursorShape_ == overlay_game::Cursor::ARROW)
    {
        Windows::OrginalApi::SetCursor(arrowCursor_);
        return true;
    }
    else if (cursorShape_ == overlay_game::Cursor::IBEAM)
    {
        Windows::OrginalApi::SetCursor(ibeamCursor_);
        return true;
    }
    else if (cursorShape_ == overlay_game::Cursor::HAND )
    {
        Windows::OrginalApi::SetCursor(handCusor_);
        return true;
    }
    else if (cursorShape_ == overlay_game::Cursor::CROSS)
    {
        Windows::OrginalApi::SetCursor(crossCusor_);
        return true;
    }
    else if (cursorShape_ == overlay_game::Cursor::WAIT)
    {
        Windows::OrginalApi::SetCursor(waitCusor_);
        return true;
    }
    else if (cursorShape_ == overlay_game::Cursor::HELP)
    {
        Windows::OrginalApi::SetCursor(helpCusor_);
        return true;
    }
    else if (cursorShape_ == overlay_game::Cursor::SIZEALL)
    {
        Windows::OrginalApi::SetCursor(sizeAllCusor_);
        return true;
    }
    else if (cursorShape_ == overlay_game::Cursor::SIZENWSE)
    {
        Windows::OrginalApi::SetCursor(sizeNWSECusor_);
        return true;
    }
    else if (cursorShape_ == overlay_game::Cursor::SIZENESW)
    {
        Windows::OrginalApi::SetCursor(sizeNESWCusor_);
        return true;
    }
    else if (cursorShape_ == overlay_game::Cursor::SIZENS)
    {
        Windows::OrginalApi::SetCursor(sizeNSCusor_);
        return true;
    }
    else if (cursorShape_ == overlay_game::Cursor::SIZEWE)
    {
        Windows::OrginalApi::SetCursor(sizeWECusor_);
        return true;
    }
    else
    {
        Windows::OrginalApi::SetCursor(arrowCursor_);
        return true;
    }
}

void OverlayConnector::clearMouseDrag()
{
    std::lock_guard<std::recursive_mutex> lock(mouseDragLock_);
    mousePressWindowId_ = 0;
    dragMoveWindowId_ = 0;
    dragMoveWindowHandle_ = 0;
    dragMoveMode_ = HTNOWHERE;
    hitTest_ = HTNOWHERE;
}

void OverlayConnector::_heartbeat()
{
    overlay::HeartBeat message;
    _sendMessage(&message);
}

void OverlayConnector::_sendGameExit()
{
    overlay::GameExit message;
    _sendMessage(&message);
}

void OverlayConnector::_sendGameProcessInfo()
{
    overlay::GameProcessInfo message;
    message.path = Storm::Utils::toUtf8(HookApp::instance()->procPath());
    _sendMessage(&message);
}

void OverlayConnector::_sendInputHookInfo(bool hooked)
{
    overlay::InputHookInfo message;
    message.hooked = hooked;
    _sendMessage(&message);
}

void OverlayConnector::_sendGraphicsHookInfo(const overlay_game::D3d9HookInfo &info)
{
    overlay::D3d9HookInfo hookInfo;
    hookInfo.endSceneHooked = info.endSceneHooked;
    hookInfo.presentHooked = info.presentHooked;
    hookInfo.presentExHooked = info.presentExHooked;
    hookInfo.swapChainPresentHooked = info.swapChainPresentHooked;
    hookInfo.resetHooked = info.resetHooked;
    hookInfo.resetExHooked = info.resetExHooked;

    overlay::GraphicsHookInfo message;
    message.graphics = "d3d9";
    message.d3d9hookInfo = hookInfo;

    _sendMessage(&message);
}

void OverlayConnector::_sendGraphicsHookInfo(const overlay_game::DxgiHookInfo &info)
{
    overlay::DxgiHookInfo hookInfo;
    hookInfo.presentHooked = info.presentHooked;
    hookInfo.present1Hooked = info.present1Hooked;
    hookInfo.resizeBufferHooked = info.resizeBufferHooked;
    hookInfo.resizeTargetHooked = info.resizeTargetHooked;

    overlay::GraphicsHookInfo message;
    message.graphics = "dxgi";
    message.dxgihookInfo = hookInfo;

    _sendMessage(&message);
}

void OverlayConnector::_sendGraphicsWindowSetupInfo(HWND window, int width, int height, bool focus, bool hooked)
{
    overlay::GraphicsWindowSetup message;
    message.window = (std::uint32_t)window;
    message.width = width;
    message.height = height;
    message.focus = focus;
    message.hooked = hooked;

    _sendMessage(&message);
}

void OverlayConnector::_sendInputIntercept(bool v)
{
    overlay::GameInputIntercept message;
    message.intercepting = v;

    _sendMessage(&message);
}

void OverlayConnector::_sendGameWindowInput(std::uint32_t windowId, UINT msg, WPARAM wparam, LPARAM lparam)
{
    overlay::GameInput message;
    message.windowId = windowId;
    message.msg = msg;
    message.wparam = wparam;
    message.lparam = lparam;

    _sendMessage(&message);
}

void OverlayConnector::_sendGraphicsWindowResizeEvent(HWND window, int width, int height)
{
    overlay::GraphicsWindowRezizeEvent message;
    message.window = (std::uint32_t)window;
    message.width = width;
    message.height = height;

    _sendMessage(&message);
}

void OverlayConnector::_sendGraphicsWindowFocusEvent(HWND window, bool focus)
{
    overlay::GraphicsWindowFocusEvent message;
    message.window = (std::uint32_t)window;
    message.focus = focus;

    _sendMessage(&message);
}

void OverlayConnector::_sendGraphicsWindowDestroy(HWND window)
{
    overlay::GraphicsWindowDestroyEvent message;
    message.window = (std::uint32_t)window;

    _sendMessage(&message);
}

void OverlayConnector::_sendGraphicsFps(std::uint32_t fps)
{
    overlay::GraphicsFps message;
    message.fps = fps;

    _sendMessage(&message);
}

void OverlayConnector::_sendMessage(overlay::GMessage *message)
{
    overlay::OverlayIpc ipcMsg;

    overlay::json obj = message->toJson();

    ipcMsg.type = message->msgType();
    ipcMsg.message = obj.dump();


    //std::cout << __FUNCTION__ << ", " << ipcMsg.type << std::endl;
    //std::cout << __FUNCTION__ << ", " << ipcMsg.message << std::endl;

    if (ipcLink_)
    {
        getIpcCenter()->sendMessage(ipcLink_, ipcClientId_, 0, &ipcMsg);
    }
}

void OverlayConnector::_onRemoteConnect()
{
    session::setOverlayConnected(true);

    this->_sendGameProcessInfo();
}

void OverlayConnector::_onRemoteClose()
{
    {
        std::lock_guard<std::mutex> lock(windowsLock_);
        windows_.clear();
    }
    {
        std::lock_guard<std::mutex> lock(framesLock_);
        frameBuffers_.clear();
    }

    shareMemoryLock_.close();

    clearMouseDrag();

    mainWindowId_ = 0;
    focusWindowId_ = 0;

    session::setOverlayEnabled(false);
    session::setOverlayConnected(false);
}

void OverlayConnector::onLinkConnect(IIpcLink *)
{
    __trace__;

    LOGGER("n_overlay") << "@trace";

    _onRemoteConnect();
}

void OverlayConnector::onLinkClose(IIpcLink *)
{
    __trace__;

    ipcLink_ = nullptr;

    _onRemoteClose();
}

void OverlayConnector::onMessage(IIpcLink * /*link*/, int /*hostPort*/, const std::string &message)
{
    int ipcMsgId = *(int *)message.c_str();
    if (ipcMsgId == (int)overlay::OverlayIpc::MsgId)
    {
        overlay::OverlayIpc ipcMsg;
        ipcMsg.upack(message);

        //std::cout << __FUNCTION__ << "," << ipcMsg.type << std::endl;

        if (ipcMsg.type == "overlay.init")
        {
            std::shared_ptr<overlay::OverlayInit> overlayMsg = std::make_shared<overlay::OverlayInit>();
            overlay::json json = overlay::json::parse(ipcMsg.message);
            overlayMsg->fromJson(json);

            _onOverlayInit(overlayMsg);
        }
        else if(ipcMsg.type == "overlay.enable")
        {
            std::shared_ptr<overlay::OverlayEnable> overlayMsg = std::make_shared<overlay::OverlayEnable>();
            overlay::json json = overlay::json::parse(ipcMsg.message);
            overlayMsg->fromJson(json);

            _onOverlayEnable(overlayMsg);
        }
        else if (ipcMsg.type == "window")
        {
            std::shared_ptr<overlay::Window> overlayMsg = std::make_shared<overlay::Window>();
            overlay::json json = overlay::json::parse(ipcMsg.message);
            overlayMsg->fromJson(json);

            _onWindow(overlayMsg);
        }
        else if (ipcMsg.type == "window.framebuffer")
        {
            std::shared_ptr<overlay::WindowFrameBuffer> overlayMsg = std::make_shared<overlay::WindowFrameBuffer>();
            overlay::json json = overlay::json::parse(ipcMsg.message);
            overlayMsg->fromJson(json);

            _onWindowFrameBuffer(overlayMsg);
        }
        else if (ipcMsg.type == "window.close")
        {
            std::shared_ptr<overlay::WindowClose> overlayMsg = std::make_shared<overlay::WindowClose>();
            overlay::json json = overlay::json::parse(ipcMsg.message);
            overlayMsg->fromJson(json);

            _onWindowClose(overlayMsg);
        }
        else if (ipcMsg.type == "window.bounds")
        {
            std::shared_ptr<overlay::WindowBounds> overlayMsg = std::make_shared<overlay::WindowBounds>();
            overlay::json json = overlay::json::parse(ipcMsg.message);
            overlayMsg->fromJson(json);

            _onWindowBounds(overlayMsg);
        }
        else if (ipcMsg.type == "command.cursor")
        {
            std::shared_ptr<overlay::CursorCommand> overlayMsg = std::make_shared<overlay::CursorCommand>();
            overlay::json json = overlay::json::parse(ipcMsg.message);
            overlayMsg->fromJson(json);

            _onCursorCommand(overlayMsg);
        }
    }
}

void OverlayConnector::saveClientId(IIpcLink * /*link*/, int clientId)
{
    __trace__;
    ipcClientId_ = clientId;
}

void OverlayConnector::_onOverlayInit(std::shared_ptr<overlay::OverlayInit>& overlayMsg)
{
    session::setOverlayEnabled(overlayMsg->processEnabled);
    if (overlayMsg->processEnabled)
    {
        HookApp::instance()->startHook();
    }

    shareMemoryLock_.open(Storm::Utils::fromUtf8(overlayMsg->shareMemMutex));

    std::vector<std::shared_ptr<overlay::Window>> windows;
    for (const auto& window : overlayMsg->windows)
    {
        windows.emplace_back(std::make_shared<overlay::Window>(window));
        if (window.transparent)
        {
            _updateFrameBuffer(window.windowId, window.bufferName);
        }

        if (window.name == "MainOverlay")
        {
            mainWindowId_ = window.windowId;
        }
    }

    std::lock_guard<std::mutex> lock(windowsLock_);
    windows_.swap(windows);
}

void OverlayConnector::_onOverlayEnable(std::shared_ptr<overlay::OverlayEnable>& overlayMsg)
{
    session::setOverlayEnabled(overlayMsg->processEnabled);
}

void OverlayConnector::_onWindow(std::shared_ptr<overlay::Window>& overlayMsg)
{
    {
        std::lock_guard<std::mutex> lock(windowsLock_);
        windows_.push_back(overlayMsg);

        if (overlayMsg->name == "MainOverlay")
        {
            mainWindowId_ = overlayMsg->windowId;
        }
    }
    if (overlayMsg->transparent)
    {
        _updateFrameBuffer(overlayMsg->windowId, overlayMsg->bufferName);
    }

    this->windowEvent()(overlayMsg->windowId);

    focusWindowId_ = overlayMsg->windowId;
    this->windowFocusEvent()(overlayMsg->windowId);
}

void OverlayConnector::_onWindowFrameBuffer(std::shared_ptr<overlay::WindowFrameBuffer>& overlayMsg)
{
    std::lock_guard<std::mutex> lock(windowsLock_);

    auto it = std::find_if(windows_.begin(), windows_.end(), [&](const auto &window) {
        return overlayMsg->windowId == window->windowId;
    });

    if (it != windows_.end())
    {
        auto window = *it;
        if (window->transparent)
        {
            _updateFrameBuffer(window->windowId, window->bufferName);
        }

        this->frameBufferEvent()(window->windowId);
    }
}

void OverlayConnector::_onWindowClose(std::shared_ptr<overlay::WindowClose>& overlayMsg)
{
    std::lock_guard<std::mutex> lock(windowsLock_);
    auto it = std::find_if(windows_.begin(), windows_.end(), [&](const auto &window) {
        return overlayMsg->windowId == window->windowId;
    });

    if (it != windows_.end())
    {
        windows_.erase(it);

        this->windowCloseEvent()(overlayMsg->windowId);

        {
            std::lock_guard<std::recursive_mutex> lock(mouseDragLock_);
            if (overlayMsg->windowId == dragMoveWindowId_)
            {
                clearMouseDrag();
            }
        }

        if (focusWindowId_ == overlayMsg->windowId )
        {
            focusWindowId_ = 0;
        }
    }
}

void OverlayConnector::_onWindowBounds(std::shared_ptr<overlay::WindowBounds>& overlayMsg)
{
    std::lock_guard<std::mutex> lock(windowsLock_);
    auto it = std::find_if(windows_.begin(), windows_.end(), [&](const auto &window) {
        return overlayMsg->windowId == window->windowId;
    });

    if (it != windows_.end())
    {
        auto& window = *it;
        window->rect = overlayMsg->rect;

        if (overlayMsg->bufferName)
        {
            window->bufferName = overlayMsg->bufferName.value();

            if (window->transparent)
            {
                _updateFrameBuffer(window->windowId, window->bufferName);
            }

            this->frameBufferUpdateEvent()(overlayMsg->windowId);
        }

        this->windowBoundsEvent()(overlayMsg->windowId, overlayMsg->rect);
    }
}

void OverlayConnector::_updateFrameBuffer(std::uint32_t windowId, const std::string &bufferName)
{
    namespace share_mem = boost::interprocess;

    std::shared_ptr<share_mem::windows_shared_memory> windowBitmapMem;
    std::shared_ptr<share_mem::mapped_region> fullRegion;

    try
    {
        windowBitmapMem.reset(new boost::interprocess::windows_shared_memory(share_mem::open_only, bufferName.c_str(), share_mem::read_only));
        fullRegion.reset(new share_mem::mapped_region(*windowBitmapMem, share_mem::read_only));
    }
    catch (...)
    {
    }

    if (fullRegion)
    {
        Storm::ScopeLovkV1 lockShareMem(shareMemoryLock_);

        char *orgin = static_cast<char *>(fullRegion->get_address());
        overlay::ShareMemFrameBuffer *head = (overlay::ShareMemFrameBuffer *)orgin;
        int *mem = (int *)(orgin + sizeof(overlay::ShareMemFrameBuffer));

        std::shared_ptr<overlay_game::FrameBuffer> frameBuffer(new overlay_game::FrameBuffer(head->width, head->height, mem));

        std::lock_guard<std::mutex> lock(framesLock_);
        frameBuffers_[windowId] = frameBuffer;

        std::cout << __FUNCTION__ << ", width:" << head->width << ", height:" << head->height << std::endl;
    }
}

void OverlayConnector::_onCursorCommand(std::shared_ptr<overlay::CursorCommand>& overlayMsg)
{
    static std::map<std::string, overlay_game::Cursor> cursorMap = {
        { "IDC_ARROW", overlay_game::Cursor::ARROW },
        { "IDC_IBEAM", overlay_game::Cursor::IBEAM },
        { "IDC_HAND", overlay_game::Cursor::HAND },
        { "IDC_CROSS", overlay_game::Cursor::CROSS },
        { "IDC_WAIT", overlay_game::Cursor::WAIT },
        { "IDC_HELP", overlay_game::Cursor::HELP },
        { "IDC_SIZEALL", overlay_game::Cursor::SIZEALL},
        { "IDC_SIZENWSE", overlay_game::Cursor::SIZENWSE },
        { "IDC_SIZENESW", overlay_game::Cursor::SIZENESW },
        { "IDC_SIZENS", overlay_game::Cursor::SIZENS },
        { "IDC_SIZEWE", overlay_game::Cursor::SIZEWE },
    };
    cursorShape_ = cursorMap[overlayMsg->cursor];
}
