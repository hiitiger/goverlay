#pragma once
#include <assert.h>
#include "ipc/ipcmsg.h"
#include "json/jsonutils.h"

//just use json
//whatever...

namespace overlay
{

struct ShareMemFrameBuffer
{
    int width;
    int height;
};

struct OverlayIpc : public IpcMsg
{
    std::string type;
    std::string message;
    enum
    {
        MsgId = 100
    };

    OverlayIpc()
    {
        msgId = MsgId;
    }

    virtual void pack(std::string &data) const
    {
        MsgPacker packer(data);
        packer.pushData(msgId);
        packer.pushData(type);
        packer.pushData(message);
    }

    virtual void upack(const std::string &data)
    {
        MsgUnpacker unpacker(data);
        unpacker.popData(msgId);
        unpacker.popData(type);
        unpacker.popData(message);
    }
};

using json = nlohmann::json;

struct Hotkey
{
    std::string name;
    int keyCode = 0;
    bool ctrl = false;
    bool shift = false;
    bool alt = false;
    bool passthrough = false;

    Hotkey() = default;
};

JSON_AUTO(Hotkey, name, keyCode, ctrl, shift, alt, passthrough)

struct WindowRect
{
    int x;
    int y;
    int width;
    int height;

    WindowRect() = default;
};

JSON_AUTO(WindowRect, x, y, width, height)

struct WindowCaptionMargin
{
    int left = 0;
    int right = 0;
    int top = 0;
    int height = 0;

    WindowCaptionMargin() = default;
};

JSON_AUTO(WindowCaptionMargin, left, right, top, height)

struct GMessage
{
    std::string type = "abstract";
    virtual std::string msgType() const { return type; }

    virtual bool fromJson(const json &obj) = 0;
    virtual json toJson(bool *ok = false) const = 0;
};

struct HeartBeat : public GMessage
{
    std::string type = "heartbeat";
    virtual std::string msgType() const { return type; }

    virtual bool fromJson(const json &obj)
    {
        assert(obj["type"].get<std::string>() == this->type);
        return true;
    }

    virtual json toJson(bool *ok = false) const
    {
        json result = {{"type", type}};
        if (ok)
            *ok = true;
        return result;
    }
};

JSON_AUTO(HeartBeat, type)

// from main to game
struct Window : public GMessage
{
    std::string type = "window";
    virtual std::string msgType() const { return type; }

    std::uint32_t windowId;
    std::uint32_t nativeHandle;
    std::string name;
    bool transparent = false;
    bool resizable = false;
    std::uint32_t dragBorderWidth = 0;

    std::string bufferName;

    WindowRect rect;
    std::optional<WindowCaptionMargin> caption;

    virtual bool fromJson(const json &obj)
    {
        assert(obj["type"].get<std::string>() == this->type);
        assert(obj["rect"].is_object());

        *this = obj;
        return true;
    }
    virtual json toJson(bool *ok = false) const
    {
        json result = *this;
        if (ok)
            *ok = true;
        return result;
    }
};

JSON_AUTO(Window, type, windowId, nativeHandle, name, transparent, resizable, dragBorderWidth, bufferName, rect, caption)

struct WindowClose : public GMessage
{
    std::string type = "window.close";
    virtual std::string msgType() const { return type; }
    std::uint32_t windowId;

    virtual bool fromJson(const json &obj)
    {
        assert(obj["type"].get<std::string>() == this->type);

        *this = obj;
        return true;
    }
    virtual json toJson(bool *ok = false) const
    {
        json result = *this;
        if (ok)
            *ok = true;
        return result;
    }
};
JSON_AUTO(WindowClose, type, windowId)

struct WindowBounds : public GMessage
{
    std::string type = "window.bounds";
    virtual std::string msgType() const { return type; }

    std::uint32_t windowId;
    WindowRect rect;

    std::optional<std::string> bufferName;

    virtual bool fromJson(const json &obj)
    {
        assert(obj["type"].get<std::string>() == this->type);

        *this = obj;
        return true;
    }
    virtual json toJson(bool *ok = false) const
    {
        json result = *this;
        if (ok)
            *ok = true;
        return result;
    }
};
JSON_AUTO(WindowBounds, type, windowId, rect, bufferName)

struct WindowFrameBuffer : public GMessage
{
    std::string type = "window.framebuffer";
    virtual std::string msgType() const { return type; }

    std::uint32_t windowId;

    virtual bool fromJson(const json &obj)
    {
        assert(obj["type"].get<std::string>() == this->type);

        *this = obj;

        return true;
    }
    virtual json toJson(bool *ok = false) const
    {
        json result = *this;

        if (ok)
            *ok = true;
        return result;
    }
};

JSON_AUTO(WindowFrameBuffer, type, windowId)

struct CursorCommand : public GMessage
{
    std::string type = "command.cursor";
    virtual std::string msgType() const { return type; }
    std::string cursor;

    virtual bool fromJson(const json &obj)
    {
        assert(obj["type"].get<std::string>() == this->type);

        *this = obj;

        return true;
    }
    virtual json toJson(bool *ok = false) const
    {
        json result = *this;

        if (ok)
            *ok = true;
        return result;
    }
};

JSON_AUTO(CursorCommand, type, cursor)

struct FpsCommand : public GMessage
{
    std::string type = "command.fps";
    virtual std::string msgType() const { return type; }

    bool showfps;
    int position;

    virtual bool fromJson(const json &obj)
    {
        assert(obj["type"].get<std::string>() == this->type);

        *this = obj;

        return true;
    }
    virtual json toJson(bool *ok = false) const
    {
        json result = *this;

        if (ok)
            *ok = true;
        return result;
    }
};

JSON_AUTO(FpsCommand, type, showfps, position)

struct OverlayInit : public GMessage
{
    std::string type = "overlay.init";
    virtual std::string msgType() const { return type; }

    bool processEnabled;
    std::string shareMemMutex;

    std::vector<Hotkey> hotkeys;
    std::vector<Window> windows;

    virtual bool fromJson(const json &obj)
    {
        assert(obj["type"].get<std::string>() == this->type);
        assert(obj["hotkeys"].is_array());
        assert(obj["windows"].is_array());

        processEnabled = obj["processEnabled"].get<bool>();
        shareMemMutex = obj["shareMemMutex"].get<std::string>();

        *this = obj;

        return true;
    }

    virtual json toJson(bool *ok = false) const
    {
        json result = *this;

        if (ok)
            *ok = true;
        return result;
    }
};

JSON_AUTO(OverlayInit, type, processEnabled, shareMemMutex, hotkeys, windows)

struct OverlayEnable : public GMessage
{
    std::string type = "overlay.enable";
    virtual std::string msgType() const { return type; }

    bool processEnabled;

    virtual bool fromJson(const json &obj)
    {
        assert(obj["type"].get<std::string>() == this->type);

        *this = obj;

        return true;
    }

    virtual json toJson(bool *ok = false) const
    {
        json result = *this;

        if (ok)
            *ok = true;
        return result;
    }
};

JSON_AUTO(OverlayEnable, type, processEnabled)

struct HotkeyInfo : public GMessage
{
    std::string type = "overlay.hotkey";
    virtual std::string msgType() const { return type; }

    std::vector<Hotkey> hotkeys;

    virtual bool fromJson(const json &obj)
    {
        assert(obj["type"].get<std::string>() == this->type);
        assert(obj["hotkeys"].is_array());

        *this = obj;

        return true;
    }

    virtual json toJson(bool *ok = false) const
    {
        json result = *this;

        if (ok)
            *ok = true;
        return result;
    }
};

JSON_AUTO(HotkeyInfo, type, hotkeys)

// from game to main

struct D3d9HookInfo
{
    bool endSceneHooked = false;
    bool presentHooked = false;
    bool presentExHooked = false;
    bool swapChainPresentHooked = false;
    bool resetHooked = false;
    bool resetExHooked = false;

    D3d9HookInfo() = default;
};

JSON_AUTO(D3d9HookInfo, endSceneHooked, presentHooked, presentExHooked, swapChainPresentHooked, resetHooked, resetExHooked)

struct DxgiHookInfo
{
    bool presentHooked = false;
    bool present1Hooked = false;
    bool resizeBufferHooked = false;
    bool resizeTargetHooked = false;

    DxgiHookInfo() = default;
};

JSON_AUTO(DxgiHookInfo, presentHooked, present1Hooked, resizeBufferHooked, resizeTargetHooked)

struct GameProcessInfo : public GMessage
{
    std::string type = "game.process";
    virtual std::string msgType() const { return type; }

    std::string path;

    virtual bool fromJson(const json &obj)
    {
        assert(obj["type"].get<std::string>() == this->type);

        *this = obj;

        return true;
    }

    virtual json toJson(bool *ok = false) const
    {
        json result = *this;

        if (ok)
            *ok = true;
        return result;
    }
};

JSON_AUTO(GameProcessInfo, type, path)

struct GameExit : public GMessage
{
    std::string type = "game.exit";
    virtual std::string msgType() const { return type; }

    virtual bool fromJson(const json &obj)
    {
        assert(obj["type"].get<std::string>() == this->type);
        *this = obj;
        return true;
    }

    virtual json toJson(bool *ok = false) const
    {
        json result = *this;
        if (ok)
            *ok = true;
        return result;
    }
};

JSON_AUTO(GameExit, type)

struct InputHookInfo : public GMessage
{
    std::string type = "input.hook";
    virtual std::string msgType() const { return type; }

    bool hooked;

    virtual bool fromJson(const json &obj)
    {
        assert(obj["type"].get<std::string>() == this->type);
        *this = obj;
        return true;
    }

    virtual json toJson(bool *ok = false) const
    {
        json result = *this;
        if (ok)
            *ok = true;
        return result;
    }
};
JSON_AUTO(InputHookInfo, type, hooked)

struct GraphicsHookInfo : public GMessage
{
    std::string type = "graphics.hook";
    virtual std::string msgType() const { return type; }

    std::string graphics;
    std::optional<D3d9HookInfo> d3d9hookInfo;
    std::optional<DxgiHookInfo> dxgihookInfo;

    virtual bool fromJson(const json &obj)
    {
        assert(obj["type"].get<std::string>() == this->type);
        this->graphics = obj["graphics"].get<std::string>();

        if (this->graphics == "d3d9")
        {
            this->d3d9hookInfo = D3d9HookInfo(obj["hookInfo"]);
        }
        else if (this->graphics == "dxgi")
        {
            this->dxgihookInfo = DxgiHookInfo(obj["hookInfo"]);
        }
        return true;
    }

    virtual json toJson(bool *ok = false) const
    {
        json result = {
            {"type", type},
            {"graphics", graphics},
        };

        if (this->graphics == "d3d9")
        {
            result["hookInfo"] = this->d3d9hookInfo.value();
        }
        else if (this->graphics == "dxgi")
        {
            result["hookInfo"] = this->dxgihookInfo.value();
        }
        if (ok)
            *ok = true;
        return result;
    }
};

inline void from_json(const json &j, GraphicsHookInfo &object)
{
    object.fromJson(j);
}

inline void to_json(json &j, const GraphicsHookInfo &object)
{
    j = object.toJson();
}

struct GraphicsWindowSetup : public GMessage
{
    std::string type = "graphics.window";
    virtual std::string msgType() const { return type; }

    std::uint32_t window;
    int width;
    int height;
    bool focus;
    bool hooked;

    virtual bool fromJson(const json &obj)
    {
        assert(obj["type"].get<std::string>() == this->type);

        *this = obj;

        return true;
    }

    virtual json toJson(bool *ok = false) const
    {
        json result = *this;

        if (ok)
            *ok = true;
        return result;
    }
};

JSON_AUTO(GraphicsWindowSetup, type, window, width, height, focus, hooked)

struct GraphicsWindowFocusEvent : public GMessage
{
    std::string type = "graphics.window.event.focus";
    virtual std::string msgType() const { return type; }
    std::uint32_t window;
    bool focus;

    virtual bool fromJson(const json &obj)
    {
        assert(obj["type"].get<std::string>() == this->type);
        *this = obj;
        return true;
    }

    virtual json toJson(bool *ok = false) const
    {
        json result = *this;

        if (ok)
            *ok = true;
        return result;
    }
};

JSON_AUTO(GraphicsWindowFocusEvent, type, window, focus)

struct GraphicsWindowRezizeEvent : public GMessage
{
    std::string type = "graphics.window.event.resize";
    virtual std::string msgType() const { return type; }
    std::uint32_t window;
    int width;
    int height;

    virtual bool fromJson(const json &obj)
    {
        assert(obj["type"].get<std::string>() == this->type);
        *this = obj;
        return true;
    }

    virtual json toJson(bool *ok = false) const
    {
        json result = *this;
        if (ok)
            *ok = true;
        return result;
    }
};

JSON_AUTO(GraphicsWindowRezizeEvent, type, window, width, height)

struct GraphicsWindowDestroyEvent : public GMessage
{
    std::string type = "graphics.window.event.destroy";
    virtual std::string msgType() const { return type; }
    std::uint32_t window;

    virtual bool fromJson(const json &obj)
    {
        assert(obj["type"].get<std::string>() == this->type);
        *this = obj;
        return true;
    }

    virtual json toJson(bool *ok = false) const
    {
        json result = *this;
        if (ok)
            *ok = true;
        return result;
    }
};

JSON_AUTO(GraphicsWindowDestroyEvent, type, window)

struct GameInputIntercept : public GMessage
{
    std::string type = "game.input.intercept";
    virtual std::string msgType() const { return type; }

    bool intercepting;

    virtual bool fromJson(const json &obj)
    {
        assert(obj["type"].get<std::string>() == this->type);

        *this = obj;

        return true;
    }

    virtual json toJson(bool *ok = false) const
    {
        json result = *this;

        if (ok)
            *ok = true;
        return result;
    }
};

JSON_AUTO(GameInputIntercept, type, intercepting)

struct GameInput : public GMessage
{
    std::string type = "game.input";
    virtual std::string msgType() const { return type; }

    std::uint32_t windowId;
    std::uint32_t msg;
    std::uint32_t wparam;
    std::uint32_t lparam;

    virtual bool fromJson(const json &obj)
    {
        assert(obj["type"].get<std::string>() == this->type);

        *this = obj;

        return true;
    }

    virtual json toJson(bool *ok = false) const
    {
        json result = *this;

        if (ok)
            *ok = true;
        return result;
    }
};

JSON_AUTO(GameInput, type, windowId, msg, wparam, lparam)

} // namespace overlay
