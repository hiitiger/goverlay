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

    std::vector<overlay::Window> windows_;

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
        this->ipcHostCenter_ = createIpcHostCenter();
    }

    void stop()
    {
        if (this->ipcHostCenter_)
        {
            destroyIpcHostCenter(this->ipcHostCenter_);
            this->ipcHostCenter_ = nullptr;
        }
    }

    Napi::Value setHotkeys(const Napi::CallbackInfo &info)
    {
        Napi::Env env = info.Env();

        std::vector<overlay::Hotkey> hotkeys;
        Napi::Array arr = info[0].As<Napi::Array>();
        for (auto i = 0; i != arr.Length(); ++i)
        {
            Napi::Object object = arr.Get(i).ToObject();
            overlay::Hotkey hotkey;
            hotkey.name = object.Get("name").ToString();
            hotkey.vKey = object.Get("vKey").ToNumber();
            hotkey.modifiers = object.Get("modifiers").ToNumber();
            hotkey.passthrough = object.Get("passthrough").ToBoolean();
            hotkeys.push_back(hotkey);
        }
        this->hotkeys_ = hotkeys;

        this->_sendHotkeys();
        return env.Undefined();
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

    Napi::Value addWindow(const Napi::CallbackInfo &info)
    {
        Napi::Env env = info.Env();

        overlay::Window message;

        message.windowId = info[0].ToNumber();

        Napi::Object windowDetails = info[1].ToObject();
        message.name = windowDetails.Get("name").ToString();
        message.transparent = windowDetails.Get("transparent").ToBoolean();
        message.resizable = windowDetails.Get("resizable").ToBoolean();
        message.bufferName = windowDetails.Get("bufferName").ToString();

        Napi::Object rect = windowDetails.Get("rect").ToObject();
        message.rect.x = rect.Get("x").ToNumber();
        message.rect.y = rect.Get("y").ToNumber();
        message.rect.width = rect.Get("width").ToNumber();
        message.rect.height = rect.Get("height").ToNumber();

        if (windowDetails.Has("caption"))
        {
            Napi::Object caption = windowDetails.Get("caption").ToObject();
            overlay::WindowCaptionMargin captionMargin;
            captionMargin.left = caption.Get("left").ToNumber();
            captionMargin.right = caption.Get("right").ToNumber();
            captionMargin.top = caption.Get("top").ToNumber();
            captionMargin.height = caption.Get("height").ToNumber();
            message.caption = captionMargin;
        }

        windows_.push_back(message);
        this->_sendMessage(&message);

        return env.Undefined();
    }

    Napi::Value sendFrameBuffer(const Napi::CallbackInfo &info)
    {
        Napi::Env env = info.Env();

        overlay::FrameBuffer message;
        message.windowId = info[0].ToNumber();
        Napi::Buffer<std::uint32_t> buffer = info[1].As<Napi::Buffer<std::uint32_t>>();
        std::int32_t width = info[2].ToNumber();
        std::int32_t height = info[3].ToNumber();
        std::uint32_t* data = buffer.Data();
        std::size_t length = buffer.Length();

        std::cout << "length :" << length << std::endl;
        std::cout << "width :" << width << std::endl;
        std::cout << "height :" << height << std::endl;


        this->_sendMessage(&message);
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

    void _sendHotkeys()
    {
        overlay::HotkeyInfo hotkeyInfoMessage;
        hotkeyInfoMessage.hotkeys = this->hotkeys_;

        this->_sendMessage(&hotkeyInfoMessage);
    }

    void _sendMessage(overlay::GMessage *message)
    {
        overlay::OverlayIpc ipcMsg;
        ipcMsg.type = message->type;

        overlay::json obj = message->toJson();

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
