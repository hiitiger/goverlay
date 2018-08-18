#pragma once
#include "./ipc/ipclink.h"

namespace overlay
{


class OverlayConnector : public IIpcClient
{
    int ipcClientId_ = 0;
    IIpcLink *ipcLink_ = nullptr;

public:
    OverlayConnector();
    ~OverlayConnector();

    void start();
    void quit();

    void sendGraphicsHookResult();
    void sendGraphicsInitResult();

    void sendGameWindowInput();
    void sendGameWindowEvent();

protected:
    void _heartbeat();

    void _sendOverlayExit();

    void _sendGameProcessInfo();

    void _sendGraphicsHookResult();

    void _sendGraphicsInitResult();

    void _sendGameWindowInput();

    void _sendGameWindowEvent();

private:
    void onIpcMessage();
    void onCommand();
    void onGraphicsCommand();

private:
    void onLinkConnect(IIpcLink *) override;
    void onLinkClose(IIpcLink *) override;
    void onMessage(IIpcLink *, int hostPort, const std::string &message) override;
    void saveClientId(IIpcLink *, int clientId) override;
};
} // namespace overlay