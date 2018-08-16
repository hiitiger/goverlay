#include <assert.h>
#include <set>
#include <memory>
#include "overlay.h"
#include "./ipc/tinyipc.h"

namespace overlay
{

class OverlayMain : public IIpcHost
{
    IIpcHostCenter *ipcHostCenter_;
    std::set<IIpcLink *> ipcClients_;

  public:
    OverlayMain()
    {
        this->start();
    }

    ~OverlayMain()
    {
        this->stop();
    }

    void start()
    {
    }

    void stop()
    {
    }

    void sendCommand()
    {
    }

    void setEventCallback()
    {
    }

    void emitEvent(const std::string &name, Napi::Object &data)
    {
    }


  private:
    void onClientConnect(IIpcLink *client) override
    {
        this->ipcClients_.insert(client);
    }
    void onClientClose(IIpcLink * client) override
    {
        this->ipcClients_.erase(client);
    }
    void onMessage(IIpcLink *link, int clientId, int hostPort, const std::string &message) override
    {

    }
};

std::shared_ptr<OverlayMain> gOverlayMain;

Napi::Value start(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();

    gOverlayMain = std::make_shared<OverlayMain>();

    return env.Undefined();
}

Napi::Value stop(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();

    gOverlayMain = nullptr;

    return env.Undefined();
}

Napi::Value setEventCallback(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    return env.Undefined();
}

Napi::Value log(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    return env.Undefined();
}

Napi::Value sendCommand(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    return env.Undefined();
}

Napi::Value sendGraphicsCommand(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    return env.Undefined();
}

Napi::Value sendFrameBuffer(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    return env.Undefined();
}


} // namespace overlay