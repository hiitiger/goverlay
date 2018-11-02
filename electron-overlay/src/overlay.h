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
#include "utils/share_mem.h"

namespace stdxx
{
    namespace
    {
        static constexpr unsigned int Fnv1aBasis = 0x811C9DC5;
        static constexpr unsigned int Fnv1aPrime = 0x01000193;
    }

    constexpr unsigned int hash(const char *s, unsigned int h = Fnv1aBasis)
    {
        return !*s
            ? h
            : hash(
                s + 1,
                static_cast<unsigned int>(
                (h ^ *s) * static_cast<unsigned long long>(Fnv1aPrime)));
    }

    constexpr unsigned int hash(const wchar_t *s, unsigned int h = Fnv1aBasis)
    {
        return !*s
            ? h
            : hash(
                s + 1,
                static_cast<unsigned int>(
                (h ^ *s) * static_cast<unsigned long long>(Fnv1aPrime)));
    }
}

struct share_memory
{
    std::string bufferName;
    std::unique_ptr<windows_shared_memory> windowBitmapMem;
    int maxWidth;
    int maxHeight;
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
        {37 , "Left"},
        {38 , "Up"},
        {39 , "Right"},
        {40 , "Down"},
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
        {97 , "1"},
        {98 , "2"},
        {99 , "3"},
        {100 , "4"},
        {101 , "5"},
        {102 , "6"},
        {103 , "7"},
        {104 , "8"},
        {105 , "9"},
        {106 , "*"},
        {107 , "+"},
        {109 , "-"},
        {110 , "."},
        {111 , "/"},
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

    if (::GetAsyncKeyState(VK_NUMLOCK) & 0x0001)
        modifiers.push_back("numLock");
    if (::GetAsyncKeyState(VK_CAPITAL) & 0x0001)
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

inline std::vector<std::string> getMouseModifiers(WPARAM wparam, LPARAM lparam)
{
    std::vector<std::string> modifiers;

    WORD vkState = GET_KEYSTATE_WPARAM(wparam);
    if (vkState & MK_CONTROL)
    {
        modifiers.push_back("control");
        if (isKeyDown(VK_LCONTROL))
            modifiers.push_back("left");
        else if (isKeyDown(VK_RCONTROL))
            modifiers.push_back("right");
    }
    if (vkState & MK_SHIFT)
    {
        modifiers.push_back("shift");
        if (isKeyDown(VK_LSHIFT))
            modifiers.push_back("left");
        else if (isKeyDown(VK_RSHIFT))
            modifiers.push_back("right");
    }
    if (isKeyDown(VK_MENU))
    {
        modifiers.push_back("alt");
        if (isKeyDown(VK_LMENU))
            modifiers.push_back("left");
        else if (isKeyDown(VK_RMENU))
            modifiers.push_back("right");
    }

    if (vkState & MK_LBUTTON)
        modifiers.push_back("leftButtonDown");
    if (vkState & MK_RBUTTON)
        modifiers.push_back("rightButtonDown");
    if (vkState & MK_MBUTTON)
        modifiers.push_back("middleButtonDown");

    if (isKeyDown(VK_LWIN) || isKeyDown(VK_RWIN))
        modifiers.push_back("meta");

    if (::GetAsyncKeyState(VK_NUMLOCK) & 0x0001)
        modifiers.push_back("numLock");
    if (::GetAsyncKeyState(VK_CAPITAL) & 0x0001)
        modifiers.push_back("capsLock");

    return modifiers;
}


class OverlayMain : public IIpcHost
{
    const std::string k_overlayIpcName = "n_overlay_1a1y2o8l0b";

    IIpcHostCenter *ipcHostCenter_;
    std::map<std::uint32_t, IIpcLink *> ipcClients_;

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

    void createImageMem(std::uint32_t windowId, std::string bufferName, int maxWidth, int maxHeight)
    {
        {
            std::cout << "create share mem:" << maxWidth << "," << maxHeight << std::endl;

            auto shareMemSize = maxWidth * maxHeight * sizeof(std::uint32_t) + sizeof(overlay::ShareMemFrameBuffer);
            std::shared_ptr<share_memory> imageMem = std::make_shared<share_memory>();
            imageMem->bufferName = bufferName;
            try
            {
                windows_shared_memory share_mem(windows_shared_memory::create_only, bufferName.c_str(), shareMemSize, windows_shared_memory::read_write);

                imageMem->windowBitmapMem = std::make_unique<windows_shared_memory>(std::move(share_mem));

                std::memset(imageMem->windowBitmapMem->get_address(), 0, imageMem->windowBitmapMem->get_size());

                imageMem->maxWidth = maxWidth;
                imageMem->maxHeight = maxHeight;
            }
            catch (...)
            {
                ;
            }

            shareMemMap_[windowId] = imageMem;
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

        Napi::Object commandInfo = info[0].ToObject();
        std::string command = commandInfo.Get("command").ToString();
        if (command == "cursor")
        {
            overlay::CursorCommand message;
            message.cursor = commandInfo.Get("cursor").ToString();

            this->_sendMessage(&message);
        }
        else if (command == "fps")
        {
            overlay::FpsCommand message;
            message.showfps = commandInfo.Get("showfps").ToBoolean();
            message.position = commandInfo.Get("position").ToNumber();

            this->_sendMessage(&message);
        }
        else if (command == "input.intercept")
        {
            std::cout << __FUNCTIONW__ << "input.intercept";
            overlay::InputInterceptCommand message;
            message.intercept = commandInfo.Get("intercept").ToBoolean();

            this->_sendMessage(&message);
        }

        return env.Undefined();
    }

    Napi::Value addWindow(const Napi::CallbackInfo &info)
    {
        std::cout << __FUNCTION__ <<std::endl;
        Napi::Env env = info.Env();

        overlay::Window message;

        message.windowId = info[0].ToNumber();

        Napi::Object windowDetails = info[1].ToObject();
        message.name = windowDetails.Get("name").ToString();
        message.nativeHandle = windowDetails.Get("nativeHandle").ToNumber();
        message.transparent = windowDetails.Get("transparent").ToBoolean();
        message.resizable = windowDetails.Get("resizable").ToBoolean();
        message.maxWidth = windowDetails.Get("maxWidth").ToNumber();
        message.maxHeight = windowDetails.Get("maxHeight").ToNumber();
        message.minWidth = windowDetails.Get("minWidth").ToNumber();
        message.minHeight = windowDetails.Get("minHeight").ToNumber();

        message.bufferName = _shareMemoryName(message.windowId);

        if (windowDetails.Has("dragBorderWidth"))
        {
            message.dragBorderWidth = windowDetails.Get("dragBorderWidth").ToNumber();
        }

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

        HMONITOR moniter = MonitorFromWindow(GetForegroundWindow(), MONITOR_DEFAULTTOPRIMARY);
        MONITORINFO moniterInfo = { 0 };
        moniterInfo.cbSize = sizeof(MONITORINFO);
        GetMonitorInfoW(moniter, &moniterInfo);
        int width = moniterInfo.rcMonitor.right - moniterInfo.rcMonitor.left;
        int height = moniterInfo.rcMonitor.bottom - moniterInfo.rcMonitor.top;
        createImageMem(message.windowId, message.bufferName, std::max(width, message.rect.width), std::max(height, message.rect.height));

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

        auto& imageMem = shareMemMap_[message.windowId];

        if (message.rect.width * message.rect.height > imageMem->maxWidth * imageMem->maxHeight)
        {
            auto it = std::find_if(windows_.begin(), windows_.end(), [windowId = message.windowId](const auto &window) {
                return windowId == window.windowId;
            });
            auto& window = *it;

            window.bufferName = _shareMemoryName(message.windowId);
            message.bufferName = window.bufferName;

            HMONITOR moniter = MonitorFromWindow(GetForegroundWindow(), MONITOR_DEFAULTTOPRIMARY);
            MONITORINFO moniterInfo = { 0 };
            moniterInfo.cbSize = sizeof(MONITORINFO);
            GetMonitorInfoW(moniter, &moniterInfo);
            int width = moniterInfo.rcMonitor.right - moniterInfo.rcMonitor.left;
            int height = moniterInfo.rcMonitor.bottom - moniterInfo.rcMonitor.top;

            createImageMem(window.windowId, window.bufferName, std::max(width, window.rect.width), std::max(height, window.rect.height));

        }
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

        assert((length == width * height));

        {
            auto it = shareMemMap_.find(message.windowId);
            if (it != shareMemMap_.end())
            {
                auto &windowBitmapMem = it->second->windowBitmapMem;

                if (windowBitmapMem)
                {
                    std::lock_guard<Windows::Mutex> lock(mutex_);

                    char *orgin = static_cast<char *>(windowBitmapMem->get_address());
                    std::memset(windowBitmapMem->get_address(), 0, windowBitmapMem->get_size());

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

        static WCHAR utf16Code = 0;
        assert(!utf16Code || (utf16Code && msg == WM_CHAR));

        if ((msg >= WM_KEYFIRST && msg <= WM_KEYLAST)
            || (msg >= WM_SYSKEYDOWN && msg <= WM_SYSDEADCHAR))
        {
            if (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN)
            {
                object.Set("type", "keyDown");
                object.Set("keyCode", getKeyCode(wparam));
            }
            else if (msg == WM_KEYUP || msg == WM_SYSKEYUP)
            {
                object.Set("type", "keyUp");
                object.Set("keyCode", getKeyCode(wparam));
            }
            else if (msg == WM_CHAR)
            {
                object.Set("type", "char");
                WCHAR code = wparam;

                if (0xD800 <= code && code <= 0xDBFF)
                {
                    utf16Code = code;
                }
                else
                {
                    std::wstring keyCode;
                    if (utf16Code && (0xDC00 <= code && code <= 0xDFFF))
                    {
                        keyCode = std::wstring(1, utf16Code);
                        keyCode.append(std::wstring(1, code));

                    }
                    else
                    {
                        keyCode = std::wstring(1, code);
                    }

                    utf16Code = 0;
                    object.Set("keyCode", Windows::toUtf8(keyCode));
                }
            }

            auto modifiersVec = getKeyboardModifiers(wparam, lparam);

            Napi::Array modifiers = Napi::Array::New(env, modifiersVec.size());

            for (auto i = 0; i != modifiersVec.size(); ++i)
            {
                modifiers.Set(i, modifiersVec[i]);
            }

            object.Set("modifiers", modifiers);
        }

        else if (msg >= WM_MOUSEFIRST && msg <= WM_MOUSELAST)
        {
            auto modifiersVec = getMouseModifiers(wparam, lparam);

            if (msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN 
                ||msg == WM_MBUTTONDOWN || msg == WM_XBUTTONDOWN
                || msg == WM_LBUTTONDBLCLK || msg == WM_RBUTTONDBLCLK
                || msg == WM_MBUTTONDBLCLK || msg == WM_XBUTTONDBLCLK
                )
            {
                object.Set("type", "mouseDown");

                if (msg == WM_LBUTTONDBLCLK || msg == WM_RBUTTONDBLCLK
                    || msg == WM_MBUTTONDBLCLK || msg == WM_XBUTTONDBLCLK)
                {
                    object.Set("clickCount", 2);
                }
                else
                {
                    object.Set("clickCount", 1);
                }

                
            }
            else if (msg == WM_LBUTTONUP || msg == WM_RBUTTONUP
                || msg == WM_MBUTTONUP || msg == WM_XBUTTONUP)
            {
                object.Set("type", "mouseUp");
                object.Set("clickCount", 1);
            }
            else if (msg == WM_MOUSEMOVE)
            {
                object.Set("type", "mouseMove");
            }
            else if (msg == WM_MOUSEWHEEL)
            {
                object.Set("type", "mouseWheel");

                int delta = GET_WHEEL_DELTA_WPARAM(wparam) / WHEEL_DELTA;
                object.Set("deltaY", delta);
                object.Set("canScroll ", true);
            }

            //for mousewheel the cord is already translated

            int x = LOWORD(lparam);
            int y = HIWORD(lparam);
            object.Set("x", x);
            object.Set("y", y);

            if (msg == WM_LBUTTONDOWN || msg == WM_LBUTTONUP || msg == WM_LBUTTONDBLCLK)
            {
                object.Set("button", "left");
            }
            else if (msg == WM_RBUTTONDOWN || msg == WM_RBUTTONUP || msg == WM_RBUTTONDBLCLK)
            {
                object.Set("button", "right");
            }
            else if (msg == WM_MBUTTONDOWN || msg == WM_MBUTTONUP || msg == WM_MBUTTONDBLCLK)
            {
                object.Set("button", "middle");
            }

            Napi::Array modifiers = Napi::Array::New(env, modifiersVec.size());

            for (auto i = 0; i != modifiersVec.size(); ++i)
            {
                modifiers.Set(i, modifiersVec[i]);
            }

            object.Set("modifiers", modifiers);

        }

        if (utf16Code)
        {
            return env.Undefined();
        }
        else
        {
            return object;
        }
    }

    void notifyGameProcess(std::uint32_t pid, std::string& path)
    {
        if (eventCallback_)
        {
            Napi::HandleScope scope(eventCallback_->env);
            Napi::Object object = Napi::Object::New(eventCallback_->env);
            object.Set("pid", Napi::Value::From(eventCallback_->env, pid));
            object.Set("path", Napi::Value::From(eventCallback_->env, path));
            eventCallback_->callback.MakeCallback(eventCallback_->receiver.Value(), { Napi::Value::From(eventCallback_->env, "game.process"), object });
        }
    }

    void notifyInputEvent(std::uint32_t pid, std::uint32_t windowId, std::uint32_t msg, std::uint32_t wparam, std::uint32_t lparam)
    {
        if (eventCallback_)
        {
            Napi::HandleScope scope(eventCallback_->env);
            Napi::Object object = Napi::Object::New(eventCallback_->env);
            object.Set("pid", Napi::Value::From(eventCallback_->env, pid));
            object.Set("windowId", Napi::Value::From(eventCallback_->env, windowId));
            object.Set("msg", Napi::Value::From(eventCallback_->env, msg));
            object.Set("wparam", Napi::Value::From(eventCallback_->env, wparam));
            object.Set("lparam", Napi::Value::From(eventCallback_->env, lparam));
            eventCallback_->callback.MakeCallback(eventCallback_->receiver.Value(), { Napi::Value::From(eventCallback_->env, "game.input"), object });
        }
    }

    void notifyInputIntercepEvent(std::uint32_t pid, bool intercepting)
    {
        if (eventCallback_)
        {
            Napi::HandleScope scope(eventCallback_->env);
            Napi::Object object = Napi::Object::New(eventCallback_->env);
            object.Set("pid", Napi::Value::From(eventCallback_->env, pid));
            object.Set("intercepting", Napi::Value::From(eventCallback_->env, intercepting));
            eventCallback_->callback.MakeCallback(eventCallback_->receiver.Value(), { Napi::Value::From(eventCallback_->env, "game.input.intercept"), Napi::Value::From(eventCallback_->env, object) });
        }
    }

    void notifyGraphicsWindow(std::uint32_t pid, std::uint32_t window, int width, int height, bool focused, bool hooked)
    {
        if (eventCallback_)
        {
            Napi::HandleScope scope(eventCallback_->env);
            Napi::Object object = Napi::Object::New(eventCallback_->env);
            object.Set("pid", Napi::Value::From(eventCallback_->env, pid));
            object.Set("nativeHandle", Napi::Value::From(eventCallback_->env, window));
            object.Set("width", Napi::Value::From(eventCallback_->env, width));
            object.Set("height", Napi::Value::From(eventCallback_->env, height));
            object.Set("focused", Napi::Value::From(eventCallback_->env, focused));
            object.Set("hooked", Napi::Value::From(eventCallback_->env, hooked));
            eventCallback_->callback.MakeCallback(eventCallback_->receiver.Value(), { Napi::Value::From(eventCallback_->env, "graphics.window"), Napi::Value::From(eventCallback_->env, object) });
        }
    }

    void notifyGraphicsWindowResize(std::uint32_t pid, std::uint32_t window, int width, int height)
    {
        if (eventCallback_)
        {
            Napi::HandleScope scope(eventCallback_->env);
            Napi::Object object = Napi::Object::New(eventCallback_->env);
            object.Set("pid", Napi::Value::From(eventCallback_->env, pid));
            object.Set("nativeHandle", Napi::Value::From(eventCallback_->env, window));
            object.Set("width", Napi::Value::From(eventCallback_->env, width));
            object.Set("height", Napi::Value::From(eventCallback_->env, height));
            eventCallback_->callback.MakeCallback(eventCallback_->receiver.Value(), { Napi::Value::From(eventCallback_->env, "graphics.window.event.resize"), Napi::Value::From(eventCallback_->env, object) });
        }
    }

    void notifyGraphicsWindowFocus(std::uint32_t pid, std::uint32_t window, bool focused)
    {
        if (eventCallback_)
        {
            Napi::HandleScope scope(eventCallback_->env);
            Napi::Object object = Napi::Object::New(eventCallback_->env);
            object.Set("pid", Napi::Value::From(eventCallback_->env, pid));
            object.Set("nativeHandle", Napi::Value::From(eventCallback_->env, window));
            object.Set("focused", Napi::Value::From(eventCallback_->env, focused));
            eventCallback_->callback.MakeCallback(eventCallback_->receiver.Value(), { Napi::Value::From(eventCallback_->env, "graphics.window.event.focus"), Napi::Value::From(eventCallback_->env, object) });
        }
    }

    void ontifyGrapicsFps(std::uint32_t pid, std::uint32_t fps)
    {
        if (eventCallback_)
        {
            Napi::HandleScope scope(eventCallback_->env);
            Napi::Object object = Napi::Object::New(eventCallback_->env);
            object.Set("pid", Napi::Value::From(eventCallback_->env, pid));
            object.Set("fps", Napi::Value::From(eventCallback_->env, fps));
            eventCallback_->callback.MakeCallback(eventCallback_->receiver.Value(), { Napi::Value::From(eventCallback_->env, "graphics.fps"), Napi::Value::From(eventCallback_->env, object) });
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

        for (auto [id, link] : this->ipcClients_)
        {
            this->ipcHostCenter_->sendMessage(link, 0, 0, &ipcMsg);
        }
    }

  private:
    void onClientConnect(IIpcLink *client) override
    {
        this->ipcClients_.insert(std::make_pair(client->remoteIdentity(), client));

        std::cout << __FUNCTION__ << "," << client->remoteIdentity() << std::endl;
    }
    void onClientClose(IIpcLink *client) override
    {
        this->ipcClients_.erase(client->remoteIdentity());
        std::cout << __FUNCTION__ << "," << client->remoteIdentity() << std::endl;
    }
    void onMessage(IIpcLink *link, int clientId, int hostPort, const std::string &message) override
    {
        int ipcMsgId = *(int *)message.c_str();

        if (ipcMsgId == (int)overlay::OverlayIpc::MsgId)
        {
            overlay::OverlayIpc ipcMsg;
            ipcMsg.upack(message);

#define OVERLAY_DISPATCH(type, Msg) \
case stdxx::hash(type):\
{\
    std::shared_ptr<overlay::Msg> overlayMsg = std::make_shared<overlay::Msg>(); \
    overlay::json json = overlay::json::parse(ipcMsg.message); \
    overlayMsg->fromJson(json); \
    _on##Msg(link->remoteIdentity(), overlayMsg); \
}\
break;

            switch (stdxx::hash(ipcMsg.type.c_str()))
            {
                OVERLAY_DISPATCH("game.process", GameProcessInfo);
                OVERLAY_DISPATCH("game.input", GameInput);
                OVERLAY_DISPATCH("game.input.intercept", GameInputIntercept);
                OVERLAY_DISPATCH("graphics.window", GraphicsWindowSetup);
                OVERLAY_DISPATCH("graphics.window.event.resize", GraphicsWindowRezizeEvent);
                OVERLAY_DISPATCH("graphics.window.event.focus", GraphicsWindowFocusEvent);
                OVERLAY_DISPATCH("graphics.fps", GraphicsFps);
            default:
                break;
            }
        }
    }

    void _sendOverlayInit(IIpcLink *link)
    {
        overlay::OverlayInit message;
        message.processEnabled = true;
        message.shareMemMutex = shareMemMutex_;
        message.windows = windows_;
        message.hotkeys = hotkeys_;

        overlay::OverlayIpc ipcMsg;
        ipcMsg.type = message.msgType();

        overlay::json obj = message.toJson();

        std::cout << __FUNCTION__ <<  obj.dump()  << std::endl;

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

    void _onGameProcessInfo(std::uint32_t pid, const std::shared_ptr<overlay::GameProcessInfo>& overlayMsg)
    {
        node_async_call::async_call([this, pid, overlayMsg]() {
            notifyGameProcess(pid, overlayMsg->path);
        });

        _sendOverlayInit(this->ipcClients_.at(pid));
    }

    void _onGameInput(std::uint32_t pid, const std::shared_ptr<overlay::GameInput>& overlayMsg)
    {
        node_async_call::async_call([this, pid, overlayMsg]() {
            notifyInputEvent(pid, overlayMsg->windowId, overlayMsg->msg, overlayMsg->wparam, overlayMsg->lparam);
        });
    }

    void _onGameInputIntercept(std::uint32_t pid, const std::shared_ptr<overlay::GameInputIntercept>& overlayMsg)
    {
        node_async_call::async_call([this, pid, overlayMsg]() {
            notifyInputIntercepEvent(pid, overlayMsg->intercepting);
        });
    }

    void _onGraphicsWindowSetup(std::uint32_t pid, const std::shared_ptr<overlay::GraphicsWindowSetup>& overlayMsg)
    {
        node_async_call::async_call([this, pid, overlayMsg]() {
            notifyGraphicsWindow(pid, overlayMsg->window, overlayMsg->width, overlayMsg->height, overlayMsg->focus, overlayMsg->hooked);
        });
    }

    void _onGraphicsWindowRezizeEvent(std::uint32_t pid, const std::shared_ptr<overlay::GraphicsWindowRezizeEvent>& overlayMsg)
    {
        node_async_call::async_call([this, pid, overlayMsg]() {
            notifyGraphicsWindowResize(pid, overlayMsg->window, overlayMsg->width, overlayMsg->height);
        });
    }

    void _onGraphicsWindowFocusEvent(std::uint32_t pid, const std::shared_ptr<overlay::GraphicsWindowFocusEvent>& overlayMsg)
    {
        node_async_call::async_call([this, pid, overlayMsg]() {
            notifyGraphicsWindowFocus(pid, overlayMsg->window, overlayMsg->focus);
        });
    }

    void _onGraphicsFps(std::uint32_t pid, const std::shared_ptr<overlay::GraphicsFps>& overlayMsg)
    {
        node_async_call::async_call([this, pid, overlayMsg]() {
            ontifyGrapicsFps(pid, overlayMsg->fps);
        });
    }
};
