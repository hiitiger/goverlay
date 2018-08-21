#pragma once
#include <napi.h>
#include "n-utils.hpp"
#include <assert.h>
#include <set>
#include <memory>
#include <iostream>
#include "ipc/tinyipc.h"
#include "message/gmessage.hpp"

#define BOOST_ALL_NO_LIB
#include <boost/interprocess/windows_shared_memory.hpp>
#include <boost/interprocess/mapped_region.hpp>
using namespace boost::interprocess;

class OverlayMain : public IIpcHost
{
    IIpcHostCenter *ipcHostCenter_;
    std::set<IIpcLink *> ipcClients_;

    std::shared_ptr<NodeEventCallback> eventCallback_;

    std::vector<overlay::Hotkey> hotkeys_;

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

    Napi::Value setHotkeys()
    {
    }

    Napi::Value log(const Napi::CallbackInfo &info)
    {
        Napi::Env env = info.Env();

        return env.Undefined();
    }

    Napi::Value setEventCallback(const Napi::CallbackInfo &info)
    {
        Napi::Env env = info.Env();

        Napi::Function callback = info[0].As<Napi::Function>();

        eventCallback_ = std::make_shared<NodeEventCallback>(env, Napi::Persistent(callback), Napi::Persistent(info.This().ToObject()));

        return env.Undefined();
    }

    Napi::Value sendCommand(const Napi::CallbackInfo &info)
    {
        Napi::Env env = info.Env();

        _makeCallback();

        return env.Undefined();
    }

    Napi::Value sendFrameBuffer(const Napi::CallbackInfo &info)
    {
        Napi::Env env = info.Env();

        overlay::FrameBuffer frameBufferMessage;
        frameBufferMessage.windowId = info[0].ToNumber();
        frameBufferMessage.bufferName = info[1].ToString();

        this->_sendMessage(&frameBufferMessage);
        return env.Undefined();
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

    void _sendMessage(overlay::GMessage* message)
    {
        overlay::OverlayIpc ipcMsg;
        ipcMsg.type = message->type;

        overlay::json obj;
        message->toJson(obj);

        ipcMsg.message = obj.dump();

        std::cout << ipcMsg.message << std::endl;

        for (auto link : this->ipcClients_)
        {
            this->ipcHostCenter_->sendMessage(link, 0, 0, &ipcMsg);
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
