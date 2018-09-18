#pragma once
#include <napi.h>
#include "utils/n-utils.h"
#include "utils/node_async_call.h"
#include <assert.h>
#include <set>
#include <memory>
#include <mutex>
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

inline bool isKeyDown(WPARAM wparam)
{
    return (GetAsyncKeyState(wparam) & 0x8000) != 0;
}

inline std::string getKeyCode(std::uint32_t key)
{
    static std::map<std::uint32_t, std::string> keyCodes = {
        {1, "LButton"},
        {2, "RButton"},
        {4 , "MButton"},
        {5 , "XButotn1"},
        {6 , "XButotn2"},
        {8 , "Backspace"},
        {9 , "Tab"},
        {13 , "Enter"},
        {16 , "Shift"},
        {17 , "Ctrl"},
        {18 , "Alt"},
        {19 , "Pause"},
        {20 , "CapsLock"},
        {27 , "Escape"},
        {32 , " "},
        {33 , "PageUp"},
        {34 , "PageDown"},
        {35 , "End"},
        {36 , "Home"},
        {37 , "ArrowLeft"},
        {38 , "ArrowUp"},
        {39 , "ArrowRight"},
        {40 , "ArrowDown"},
        {45 , "Insert"},
        {46 , "Delete"},
        {48 , "0"},
        {49 , "1"},
        {50 , "2"},
        {51 , "3"},
        {52 , "4"},
        {53 , "5"},
        {54 , "6"},
        {55 , "7"},
        {56 , "8"},
        {57 , "9"},
        {65 , "A"},
        {66 , "B"},
        {67 , "C"},
        {68 , "D"},
        {69 , "E"},
        {70 , "F"},
        {71 , "G"},
        {72 , "H"},
        {73 , "I"},
        {74 , "J"},
        {75 , "K"},
        {76 , "L"},
        {77 , "M"},
        {78 , "N"},
        {79 , "O"},
        {80 , "P"},
        {81 , "Q"},
        {82 , "R"},
        {83 , "S"},
        {84 , "T"},
        {85 , "U"},
        {86 , "V"},
        {87 , "W"},
        {88 , "X"},
        {89 , "Y"},
        {90 , "Z"},
        {91 , "Meta" },
        {92 , "Meta"},
        {93 , "ContextMenu"},
        {96 , "0"},
        {97 , " 1"},
        {98 , " 2"},
        {99 , " 3"},
        {100 , " 4"},
        {101 , " 5"},
        {102 , " 6"},
        {103 , " 7"},
        {104 , " 8"},
        {105 , " 9"},
        {106 , " *"},
        {107 , " +"},
        {109 , " -"},
        {110 , " ."},
        {111 , " /"},
        {112 , "F1"},
        {113 , "F2"},
        {114 , "F3"},
        {115 , "F4"},
        {116 , "F5"},
        {117 , "F6"},
        {118 , "F7"},
        {119 , "F8"},
        {120 , "F9"},
        {121 , "F10"},
        {122 , "F11"},
        {123 , "F12"},
        {144 , "NumLock"},
        {145 , "ScrollLock" },
    { 160, "Shift" },
    { 161, "Shift" },
    { 162, "Control" },
    { 163, "Control" },
    { 164, "Alt" },
    {165, "Alt"},
        {182 , "My Computer"},
        {183 , "My Calculator"},
        {186 , ";"},
        {187 , "="},
        {188 , "},"},
        {189 , "-"},
        {190 , "."},
        {191 , "/"},
        {192 , "`"},
        {219 , "["},
        {220 , "\\"},
        {221 , "]"},
        { 222 , "'" },
        { 250 , "Play" },
    };

    return keyCodes[key];
}

inline std::vector<std::string> getKeyboardModifiers(WPARAM wparam, LPARAM lparam)
{
    std::vector<std::string> modifiers;
    if (isKeyDown(VK_SHIFT))
        modifiers.push_back("shift");
    if (isKeyDown(VK_CONTROL))
        modifiers.push_back("control");
    if (isKeyDown(VK_MENU))
        modifiers.push_back("alt");
    if (isKeyDown(VK_LWIN) || isKeyDown(VK_RWIN))
        modifiers.push_back("meta");

    if (::GetAsyncKeyState(VK_NUMLOCK) & 1)
        modifiers.push_back("numLock");
    if (::GetAsyncKeyState(VK_CAPITAL) & 1)
        modifiers.push_back("capsLock");

    switch (wparam) {
    case VK_RETURN:
        if ((lparam >> 16) & KF_EXTENDED)
            modifiers.push_back("isKeypad");
        break;
    case VK_INSERT:
    case VK_DELETE:
    case VK_HOME:
    case VK_END:
    case VK_PRIOR:
    case VK_NEXT:
    case VK_UP:
    case VK_DOWN:
    case VK_LEFT:
    case VK_RIGHT:
        if (!((lparam >> 16) & KF_EXTENDED))
            modifiers.push_back("isKeypad");
        break;
    case VK_NUMLOCK:
    case VK_NUMPAD0:
    case VK_NUMPAD1:
    case VK_NUMPAD2:
    case VK_NUMPAD3:
    case VK_NUMPAD4:
    case VK_NUMPAD5:
    case VK_NUMPAD6:
    case VK_NUMPAD7:
    case VK_NUMPAD8:
    case VK_NUMPAD9:
    case VK_DIVIDE:
    case VK_MULTIPLY:
    case VK_SUBTRACT:
    case VK_ADD:
    case VK_DECIMAL:
    case VK_CLEAR:
        modifiers.push_back("isKeypad");
        break;
    case VK_SHIFT:
        if (isKeyDown(VK_LSHIFT))
            modifiers.push_back("left");
        else if (isKeyDown(VK_RSHIFT))
            modifiers.push_back("right");
        break;
    case VK_CONTROL:
        if (isKeyDown(VK_LCONTROL))
            modifiers.push_back("left");
        else if (isKeyDown(VK_RCONTROL))
            modifiers.push_back("right");
        break;
    case VK_MENU:
        if (isKeyDown(VK_LMENU))
            modifiers.push_back("left");
        else if (isKeyDown(VK_RMENU))
            modifiers.push_back("right");
        break;
    case VK_LWIN:
        modifiers.push_back("left");
        break;
    case VK_RWIN:
        modifiers.push_back("right");
        break;
    }
    return modifiers;
}



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
            hotkey.keyCode = object.Get("keyCode").ToNumber();
            if (object.Has("modifiers"))
            {
                Napi::Object modifiers = object.Get("modifiers").ToObject();
                if (modifiers.Has("ctrl"))
                {
                    hotkey.ctrl = modifiers.Get("ctrl").ToBoolean();
                }
                if (modifiers.Has("shift"))
                {
                    hotkey.shift = modifiers.Get("shift").ToBoolean();
                }
                if (modifiers.Has("alt"))
                {
                    hotkey.alt = modifiers.Get("alt").ToBoolean();
                }
            }
            if (object.Has("passthrough"))
            {
                hotkey.passthrough = object.Get("passthrough").ToBoolean();
            }
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
        message.nativeHandle = windowDetails.Get("nativeHandle").ToNumber();
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

            HMONITOR moniter = MonitorFromWindow(GetForegroundWindow(), MONITOR_DEFAULTTOPRIMARY);
            MONITORINFO moniterInfo = { 0 };
            moniterInfo.cbSize = sizeof(MONITORINFO);
            GetMonitorInfoW(moniter, &moniterInfo);
            int width = moniterInfo.rcMonitor.right - moniterInfo.rcMonitor.left;
            int height = moniterInfo.rcMonitor.bottom- moniterInfo.rcMonitor.top;
            std::cout << "create share mem:" << moniter<< ", " << width << "," << height << std::endl;

            auto shareMemSize = width * height * sizeof(std::uint32_t) + sizeof(overlay::ShareMemFrameBuffer);
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

    Napi::Value closeWindow(const Napi::CallbackInfo& info)
    {
        Napi::Env env = info.Env();

        overlay::WindowClose message;
        message.windowId = info[0].ToNumber();
        this->_sendMessage(&message);

        auto it = std::find_if(windows_.begin(), windows_.end(), [windowId = message.windowId](const auto &window) {
            return windowId == window.windowId;
        });

        if (it != windows_.end())
        {
            windows_.erase(it);
        }

        return env.Undefined();
    }

    Napi::Value sendWindowBounds(const Napi::CallbackInfo& info)
    {
        Napi::Env env = info.Env();

        overlay::WindowBounds message;
        message.windowId = info[0].ToNumber();
        Napi::Object rect = info[1].ToObject().Get("rect").ToObject();
        message.rect.x = rect.Get("x").ToNumber();
        message.rect.y = rect.Get("y").ToNumber();
        message.rect.width = rect.Get("width").ToNumber();
        message.rect.height = rect.Get("height").ToNumber();
        this->_sendMessage(&message);

        auto it = std::find_if(windows_.begin(), windows_.end(), [windowId = message.windowId](const auto &window) {
            return windowId == window.windowId;
        });

        if (it != windows_.end())
        {
            auto& window = *it;
            window.rect = message.rect;
        }

        return env.Undefined();
    }

    Napi::Value sendFrameBuffer(const Napi::CallbackInfo &info)
    {
        Napi::Env env = info.Env();

        overlay::WindowFrameBuffer message;
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
                    std::lock_guard<Windows::Mutex> lock(mutex_);

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
                }
            }
        }

        this->_sendMessage(&message);
        return env.Undefined();
    }

    Napi::Value translateInputEvent(const Napi::CallbackInfo &info)
    {
        Napi::Env env = info.Env();
        Napi::Object object = Napi::Object::New(env);
        Napi::Object eventData = info[0].ToObject();

        std::uint32_t msg = eventData.Get("msg").ToNumber();
        std::uint32_t wparam = eventData.Get("wparam").ToNumber();
        std::uint32_t lparam = eventData.Get("lparam").ToNumber();

        std::cout << "msg:" << msg << ", wparam:" << wparam << ", lparam: " << lparam << std::endl;

        if ((msg >= WM_KEYFIRST && msg <= WM_KEYLAST)
            || (msg >= WM_SYSKEYDOWN && msg <= WM_SYSDEADCHAR))
        {
            if (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN)
            {
                object.Set("type", "keyDown");
            }
            else if (msg == WM_KEYUP || msg == WM_SYSKEYUP)
            {
                object.Set("type", "keyUp");
            }
            else
            {
                object.Set("type", "char");
            }

            auto modifiersVec = getKeyboardModifiers(wparam, lparam);

            Napi::Array modifiers = Napi::Array::New(env, modifiersVec.size());

            for (auto i = 0; i != modifiersVec.size(); ++i)
            {
                modifiers.Set(i, modifiersVec[i]);
            }

            object.Set("modifiers", modifiers);
            object.Set("keyCode", getKeyCode(wparam));
        }

        else if (msg >= WM_MOUSEFIRST && msg <= WM_MOUSELAST)
        {
        }

        return object;
    }

    void notifyInputEvent(std::uint32_t windowId, std::uint32_t msg, std::uint32_t wparam, std::uint32_t lparam)
    {
        if (eventCallback_)
        {
            Napi::HandleScope scope(eventCallback_->env);
            Napi::Object object = Napi::Object::New(eventCallback_->env);
            object.Set("windowId", Napi::Value::From(eventCallback_->env, windowId));
            object.Set("msg", Napi::Value::From(eventCallback_->env, msg));
            object.Set("wparam", Napi::Value::From(eventCallback_->env, wparam));
            object.Set("lparam", Napi::Value::From(eventCallback_->env, lparam));
            eventCallback_->callback.MakeCallback(eventCallback_->receiver.Value(), { Napi::Value::From(eventCallback_->env, "game.input"), object });
        }
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
            else if (ipcMsg.type == "game.input")
            {
                std::shared_ptr<overlay::GameInput> overlayMsg = std::make_shared<overlay::GameInput>();
                overlay::json json = overlay::json::parse(ipcMsg.message);
                overlayMsg->fromJson(json);

                _onGameInput(overlayMsg);
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

    void _onGameInput(const std::shared_ptr<overlay::GameInput>& overlayMsg)
    {
        node_async_call::async_call([this, overlayMsg]() {
            notifyInputEvent(overlayMsg->windowId, overlayMsg->msg, overlayMsg->wparam, overlayMsg->lparam);
        });
    }
};
