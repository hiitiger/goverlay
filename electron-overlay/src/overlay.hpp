#pragma once
#include <napi.h>
#include "n-utils.hpp"
#include <assert.h>
#include <set>
#include <memory>
#include <iostream>
#include "./ipc/tinyipc.h"

class OverlayMain : public IIpcHost
{
    IIpcHostCenter *ipcHostCenter_;
    std::set<IIpcLink *> ipcClients_;

    std::shared_ptr<NodeEventCallback> eventCallback_;

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
        _makeCallback();
    }

    void setEventCallback(Napi::Env env, Napi::FunctionReference &&callback, Napi::ObjectReference &&receiver)
    {
        eventCallback_ = std::make_shared<NodeEventCallback>(env, std::move(callback), std::move(receiver));
    }

  private:
    void _makeCallback()
    {
        if (eventCallback_)
        {
            Napi::HandleScope scope(eventCallback_->env);
            Napi::Object object = Napi::Object::New(eventCallback_->env);
            //test
            object.Set("test", Napi::Value::From(eventCallback_->env, 123789));
            eventCallback_->callback.Call(eventCallback_->receiver.Value(), {object});
        }
    }

  private:
    void onClientConnect(IIpcLink *client) override
    {
        this->ipcClients_.insert(client);
    }
    void onClientClose(IIpcLink *client) override
    {
        this->ipcClients_.erase(client);
    }
    void onMessage(IIpcLink *link, int clientId, int hostPort, const std::string &message) override
    {
    }
};
