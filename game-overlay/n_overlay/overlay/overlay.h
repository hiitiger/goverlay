#pragma once
#include "./ipc/ipclink.h"
#include "session.h"

class OverlayConnector : public IIpcClient
{
    int ipcClientId_ = 0;
    IIpcLink *ipcLink_ = nullptr;

    Storm::Mutex shareMemoryLock_;

    std::mutex windowsLock_;
    std::vector<std::shared_ptr<overlay::Window>> windows_;
    volatile std::uint32_t mainWindowId_ = 0;
    std::atomic<std::uint32_t> focusWindowId_ = 0;

    std::mutex framesLock_;
    std::map<std::uint32_t, std::shared_ptr<overlay_game::FrameBuffer>> frameBuffers_;

    Storm::Event<void(std::uint32_t)> windowEvent_;
    Storm::Event<void(std::uint32_t)> frameBufferEvent_;
    Storm::Event<void(std::uint32_t)> windowCloseEvent_;
    Storm::Event<void(std::uint32_t, overlay::WindowRect)> windowBoundsEvent_;
    Storm::Event<void(std::uint32_t)> frameBufferUpdateEvent_;
    Storm::Event<void(std::uint32_t)> windowFocusEvent_;

    Storm::Event<void(std::vector<overlay::Hotkey>)> hotkeysEvent_;

    std::wstring mainProcessDir_;

    std::atomic<std::uint32_t> mousePressWindowId_ = 0;

    std::recursive_mutex mouseDragLock_;
    std::atomic<std::uint32_t> dragMoveWindowId_ = 0;
    std::uint32_t dragMoveWindowHandle_ = 0;
    POINT dragMoveLastMousePos_ = {0};
    std::uint32_t dragMoveMode_ = HTNOWHERE;

    std::int32_t hitTest_ = HTNOWHERE;
    std::atomic<overlay_game::Cursor> cursorShape_ = overlay_game::Cursor::ARROW;

    HCURSOR arrowCursor_ = nullptr ;
    HCURSOR ibeamCursor_ = nullptr;
    HCURSOR handCusor_ = nullptr;
    HCURSOR crossCusor_ = nullptr;
    HCURSOR waitCusor_ = nullptr;
    HCURSOR helpCusor_ = nullptr;
    HCURSOR sizeAllCusor_ = nullptr;
    HCURSOR sizeNWSECusor_ = nullptr;
    HCURSOR sizeNESWCusor_ = nullptr;
    HCURSOR sizeNSCusor_ = nullptr;
    HCURSOR sizeWECusor_ = nullptr;

public:
    OverlayConnector();
    ~OverlayConnector();

    void start();
    void quit();

    void sendInputHookInfo(bool hooked);

    void sendGraphicsHookInfo(const overlay_game::D3d9HookInfo& info);
    void sendGraphicsHookInfo(const overlay_game::DxgiHookInfo& info);

    void sendGraphicsWindowSetupInfo(HWND window, int width, int height, bool focus, bool hooked);
    void sendGraphicsWindowResizeEvent(HWND window, int width, int height);
    void sendGraphicsWindowFocusEvent(HWND window, bool focus);
    void sendGraphicsWindowDestroy(HWND window);

    void sendGraphicsFps(std::uint32_t fps);

    void sendInputIntercept();
    void sendInputStopIntercept();

    const std::vector<std::shared_ptr<overlay::Window>>& windows();

    Storm::Event<void(std::uint32_t)>& windowEvent() { return windowEvent_; }
    Storm::Event<void(std::uint32_t)>& frameBufferEvent() { return frameBufferEvent_; }
    Storm::Event<void(std::uint32_t)>& windowCloseEvent() { return windowCloseEvent_; }
    Storm::Event<void(std::uint32_t, overlay::WindowRect)>& windowBoundsEvent() { return windowBoundsEvent_; }
    Storm::Event<void(std::uint32_t)>& frameBufferUpdateEvent() { return frameBufferUpdateEvent_; }
    Storm::Event<void(std::uint32_t)>& windowFocusEvent() { return windowFocusEvent_; }
    Storm::Event<void(std::vector<overlay::Hotkey>)>& hotkeysEvent() { return hotkeysEvent_;}

    std::wstring mainProcessDir() const { return mainProcessDir_; }
    std::wstring d3dcompiler47Path() const { return L""; }
    std::wstring d3dx9Path() const { return L""; }

    void lockShareMem();
    void unlockShareMem();

    void lockWindows();
    void unlockWindows();

    bool processNCHITTEST(UINT message, WPARAM wParam, LPARAM lParam);
    bool processMouseMessage(UINT message, WPARAM wParam, LPARAM lParam);
    bool processkeyboardMessage(UINT message, WPARAM wParam, LPARAM lParam);

    bool processSetCursor();

    void clearMouseDrag();

protected:
    void _heartbeat();

    void _sendGameExit();

    void _sendGameProcessInfo();

    void _sendInputHookInfo(bool hooked);

    void _sendGraphicsHookInfo(const overlay_game::D3d9HookInfo&);
    void _sendGraphicsHookInfo(const overlay_game::DxgiHookInfo&);

    void _sendGraphicsWindowSetupInfo(HWND window, int width, int height, bool focus, bool hooked);

    void _sendInputIntercept(bool v);

    void _sendGameWindowInput(std::uint32_t windowId, UINT message, WPARAM wParam, LPARAM lParam);

    void _sendGraphicsWindowResizeEvent(HWND window, int width, int height);
    void _sendGraphicsWindowFocusEvent(HWND window, bool focus);
    void _sendGraphicsWindowDestroy(HWND window);

    void _sendGraphicsFps(std::uint32_t fps);

    void _sendMessage(overlay::GMessage* message);

private:
    void _onRemoteConnect();
    void _onRemoteClose();

private:
    void onLinkConnect(IIpcLink *) override;
    void onLinkClose(IIpcLink *) override;
    void onMessage(IIpcLink *, int hostPort, const std::string &message) override;
    void saveClientId(IIpcLink *, int clientId) override;

private:
    void _onOverlayInit(std::shared_ptr<overlay::OverlayInit>& overlayMsg);
    void _onOverlayEnable(std::shared_ptr<overlay::OverlayEnable>& overlayMsg);
    void _onWindow(std::shared_ptr<overlay::Window>& overlayMsg);
    void _onWindowFrameBuffer(std::shared_ptr<overlay::WindowFrameBuffer>& overlayMsg);

    void _onWindowClose(std::shared_ptr<overlay::WindowClose>& overlayMsg);
    void _onWindowBounds(std::shared_ptr<overlay::WindowBounds>& overlayMsg);

    void _updateFrameBuffer(std::uint32_t windowId, const std::string& bufferName);

    void _onCursorCommand(std::shared_ptr<overlay::CursorCommand>& overlayMsg);
};