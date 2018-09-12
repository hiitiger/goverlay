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

    std::mutex framesLock_;
    std::map<std::uint32_t, std::shared_ptr<overlay_game::FrameBuffer>> frameBuffers_;

    Storm::Event<void(std::uint32_t)> windowEvent_;
    Storm::Event<void(std::uint32_t)> frameBufferEvent_;

    std::wstring mainProcessDir_;

public:
    OverlayConnector();
    ~OverlayConnector();

    void start();
    void quit();

    void sendInputHookInfo();

    void sendGraphicsHookInfo(const overlay_game::D3d9HookInfo& info);
    void sendGraphicsHookInfo(const overlay_game::DxgiHookInfo& info);

    void sendGraphicsWindowSetupInfo(HWND window, int width, int height, bool focus, bool hooked);
    void sendGraphicsWindowResizeEvent(HWND window, int width, int height);
    void sendGraphicsWindowFocusEvent(HWND window, bool focus);

    void sendInputIntercept();
    void sendInputStopIntercept();

    void sendGameWindowInput();

    const std::vector<std::shared_ptr<overlay::Window>>& windows();

    Storm::Event<void(std::uint32_t)>& windowEvent() { return windowEvent_; }
    Storm::Event<void(std::uint32_t)>& frameBufferEvent() { return frameBufferEvent_; }

    std::wstring mainProcessDir() const { return mainProcessDir_; }

    void lockShareMem();
    void unlockShareMem();

    void lockWindows();
    void unlockWindows();


protected:
    void _heartbeat();

    void _sendGameExit();

    void _sendGameProcessInfo();

    void _sendInputHookInfo();

    void _sendGraphicsHookInfo(const overlay_game::D3d9HookInfo&);
    void _sendGraphicsHookInfo(const overlay_game::DxgiHookInfo&);

    void _sendGraphicsWindowSetupInfo(HWND window, int width, int height, bool focus, bool hooked);

    void _sendInputIntercept(bool v);

    void _sendGameWindowInput();

    void _sendGraphicsWindowResizeEvent(HWND window, int width, int height);
    void _sendGraphicsWindowFocusEvent(HWND window, bool focus);

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
    void _onWindowFrameBuffer(std::shared_ptr<overlay::FrameBuffer>& overlayMsg);

    void _updateFrameBuffer(std::uint32_t windowId, const std::string& bufferName);
};