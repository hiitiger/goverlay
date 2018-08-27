#pragma once
#include "./ipc/ipclink.h"


class OverlayConnector : public IIpcClient
{
    int ipcClientId_ = 0;
    IIpcLink *ipcLink_ = nullptr;

public:
    OverlayConnector();
    ~OverlayConnector();

    void start();
    void quit();

    void sendInputHookInfo();

    void sendGraphicsHookInfo(const D3d9HookInfo& info);
    void sendGraphicsHookInfo(const DxgiHookInfo& info);

    void sendGraphicsWindowSetupInfo();

    void sendInputIntercept();
    void sendInputStopIntercept();

    void sendGameWindowInput();
    void sendGameWindowResizeEvent();

protected:
    void _heartbeat();

    void _sendOverlayExit();

    void _sendGameProcessInfo();

    void _sendInputHookInfo();

    void _sendGraphicsHookInfo(const D3d9HookInfo&);
    void _sendGraphicsHookInfo(const DxgiHookInfo&);

    void _sendGraphicsWindowSetupInfo();

    void _sendInputIntercept(bool v);

    void _sendGameWindowInput();
    void _sendGameWindowResizeEvent();

    void _sendMessage(overlay::GMessage* message);

private:
    void onIpcMessage();
    void onFrameBuffer();
    void onCommand();
    void onGraphicsCommand();

private:
    void onLinkConnect(IIpcLink *) override;
    void onLinkClose(IIpcLink *) override;
    void onMessage(IIpcLink *, int hostPort, const std::string &message) override;
    void saveClientId(IIpcLink *, int clientId) override;
};