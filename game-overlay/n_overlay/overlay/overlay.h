#pragma once
#include "./ipc/ipclink.h"

namespace overlay
{
class OverlayConnector : public IIpcClient
{
    int ipcClientId_ = 0;
    IIpcLink* ipcLink_ = nullptr;

  public:
    OverlayConnector();
    ~OverlayConnector();

    void start();
    void stop();

    void heartbeat();

    void sendGameExit();

    void sendGameProcessInfo();
    void sendGameHookResult();

    void sendGameWindowInput();
    void sendGameWindowEvent();

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