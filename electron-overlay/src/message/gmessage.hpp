#include <assert.h>
#include "json/jsonutils.h"
#include "ipc/ipcmsg.h"

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
    int vKey;
    int modifiers;
    bool passthrough;
    std::string name;

    Hotkey() = default;
};

JSON_AUTO(Hotkey, vKey, modifiers, passthrough, name)

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
    int left;
    int right;
    int top;
    int height;

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
    std::string name;
    bool transparent = false;
    bool resizable = false;

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

JSON_AUTO(Window, type, windowId, name, transparent, resizable, bufferName, rect, caption)

struct FrameBuffer : public GMessage
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

JSON_AUTO(FrameBuffer, type, windowId)

struct CursorCommand : public GMessage
{
    std::string type = "command.cursor";
    virtual std::string msgType() const { return type; }

    std::string cursor;
};

JSON_AUTO(CursorCommand, type, cursor)

struct FpsCommand : public GMessage
{
    std::string type = "command.fps";
    virtual std::string msgType() const { return type; }

    bool showfps;
    int position;
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

struct MouseInput
{
};
JSON_AUTO(MouseInput)

struct KeyInput
{
};
JSON_AUTO(KeyInput)

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

struct GraphcisWindowFocusEvent : public GMessage
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

JSON_AUTO(GraphcisWindowFocusEvent, type, window, focus)

struct GraphcisWindowRezizeEvent : public GMessage
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

JSON_AUTO(GraphcisWindowRezizeEvent, type, window, width, height)

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
};

} // namespace overlay
