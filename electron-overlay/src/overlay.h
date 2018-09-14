#pragma once
#include <napi.h>
#include "utils/n-utils.h"
#include <assert.h>
#include <set>
#include <memory>
#include <iostream>
#include "ipc/tinyipc.h"
#include "message/gmessage.hpp"
#include "utils/win-utils.h"

#define BOOST_ALL_NO_LIB
#include <boost/interprocess/windows_shared_memory.hpp>
#include <boost/interprocess/mapped_region.hpp>
namespace share_mem = boost::interprocess;

struct share_memory
{
    std::string bufferName;
    std::unique_ptr<share_mem::windows_shared_memory> windowBitmapMem;
    std::unique_ptr<share_mem::mapped_region> fullRegion;
};

class OverlayMain : public IIpcHost
{
    const std::string k_overlayIpcName = "n_overlay_1a1y2o8l0b";

    IIpcHostCenter *ipcHostCenter_;
    std::set<IIpcLink *> ipcClients_;

    std::shared_ptr<NodeEventCallback> eventCallback_;

    std::vector<overlay::Hotkey> hotkeys_;

    std::vector<overlay::Window> windows_;
    std::map<std::uint32_t, std::shared_ptr<share_memory>> shareMemMap_;

    Windows::Mutex mutex_;
    std::string shareMemMutex_;

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

        ipcHostCenter_->init(k_overlayIpcName, this);

        static long long pid = GetCurrentProcessId();
        static long long time = GetTickCount();
        std::wstring name(L"electron-overlay-sharemem-{4C4BD948-0F75-413F-9667-AC64A7944D8E}");
        name.append(std::to_wstring(pid)).append(L"-").append(std::to_wstring(time));
        shareMemMutex_ = Windows::toUtf8(name);
        mutex_.create(false, name.c_str());
    }

    void stop()
    {
        mutex_.close();

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
        message.bufferName = _shareMemoryName(message.windowId);

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

        {
            share_mem::permissions perm;
            perm.set_unrestricted();

            auto shareMemSize = message.rect.width * message.rect.height * sizeof(std::uint32_t) + sizeof(overlay::ShareMemFrameBuffer);
            std::shared_ptr<share_memory> imageMem = std::make_shared<share_memory>();
            imageMem->bufferName = message.bufferName;
            try
            {
                imageMem->windowBitmapMem.reset(new share_mem::windows_shared_memory(share_mem::create_only, message.bufferName.c_str(), share_mem::read_write, shareMemSize, perm));
                imageMem->fullRegion.reset(new share_mem::mapped_region(*(imageMem->windowBitmapMem), share_mem::read_write));
            }
            catch (...)
            {
                ;
            }

            shareMemMap_.insert(std::make_pair(message.windowId, imageMem));
        }

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
        std::uint32_t *data = buffer.Data();
        std::size_t length = buffer.Length();

        std::cout << "length :" << length << ", width :" << width << ",height :" << height << std::endl;

        assert((length == width * height));

        {
            auto it = shareMemMap_.find(message.windowId);
            if (it != shareMemMap_.end())
            {
                auto bufferName = (it->second)->bufferName;

                auto &windowBitmapMem = it->second->windowBitmapMem;
                auto &fullRegion = it->second->fullRegion;

                if (fullRegion)
                {
                    mutex_.lock();

                    char *orgin = static_cast<char *>(fullRegion->get_address());
                    std::memset(fullRegion->get_address(), 0, fullRegion->get_size());

                    overlay::ShareMemFrameBuffer *head = (overlay::ShareMemFrameBuffer *)orgin;
                    head->width = width;
                    head->height = height;
                    std::uint32_t *mem = (std::uint32_t *)(orgin + sizeof(overlay::ShareMemFrameBuffer));

                    for (int i = 0; i != height; ++i)
                    {
                        const std::uint32_t *line = data + i * width;
                        int xx = i * width;
                        memcpy((mem + xx), line, sizeof(std::uint32_t) * width);
                    }

                    mutex_.unlock();
                }
            }
        }

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
        ipcMsg.type = message->msgType();

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

        std::cout << __FUNCTION__ << "," << client->remoteIdentity() << std::endl;
    }
    void onClientClose(IIpcLink *client) override
    {
        this->ipcClients_.erase(client);
        std::cout << __FUNCTION__ << "," << client->remoteIdentity() << std::endl;
    }
    void onMessage(IIpcLink *link, int clientId, int hostPort, const std::string &message) override
    {
        int ipcMsgId = *(int *)message.c_str();

        if (ipcMsgId == overlay::OverlayIpc::MsgId)
        {
            overlay::OverlayIpc ipcMsg;
            ipcMsg.upack(message);

            std::cout << __FUNCTION__ << "," << ipcMsg.type << std::endl;

            if (ipcMsg.type == "game.process")
            {
                _sendOverlayInit(link);
            }
        }
    }

    void _sendOverlayInit(IIpcLink *link)
    {
        overlay::OverlayInit message;
        message.processEnabled = true;
        message.shareMemMutex = shareMemMutex_;
        message.windows = windows_;

        overlay::OverlayIpc ipcMsg;
        ipcMsg.type = message.msgType();

        overlay::json obj = message.toJson();

        ipcMsg.message = obj.dump();
        this->ipcHostCenter_->sendMessage(link, 0, 0, &ipcMsg);
    }

    std::string _shareMemoryName(std::int32_t windowId)
    {
        static long long pid = GetCurrentProcessId();
        static long long nextImage = 0;
        long long time = GetTickCount();
        std::string name = std::string("electron-overlay-").append(std::to_string(pid)).append("-").append(std::to_string(time)).append("-");
        name.append(std::to_string((long long)windowId)).append("-image-").append(std::to_string(++nextImage));
        return name;
    }
};
