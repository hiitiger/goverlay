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
    void sendGraphicsHookInfo(const std::map<std::string, std::string>& data);
    void sendGraphicsWindowSetupInfo();

    void sendInputBlocked();
    void sendInputUnBlocked();

    void sendGameWindowInput();
    void sendGameWindowEvent();

protected:
    void _heartbeat();

    void _sendOverlayExit();

    void _sendGameProcessInfo();

    void _sendInputHookInfo();
    void _sendGraphicsHookInfo();
    void _sendGraphicsWindowSetupInfo();

    void _sendInputBlocked();
    void _sendInputUnBlocked();

    void _sendGameWindowInput();
    void _sendGameWindowEvent();

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