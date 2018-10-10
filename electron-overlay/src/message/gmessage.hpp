#pragma once
#include <assert.h>
#include "ipc/ipcmsg.h"
#include "json/jsonutils.h"

//just use json
//whatever...

#define GMESSAGE_AUTO(TYPE)\
std::string type = TYPE;\
virtual std::string msgType() const { return type; }\
virtual bool fromJson(const json &obj)\
{\
    assert(obj["type"].get<std::string>() == this->type);\
    *this = obj;\
    return true;\
}\
\
virtual json toJson(bool *ok = false) const\
{\
    json result = *this;\
    if (ok)\
        *ok = true;\
    return result;\
}

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


enum class FpsPosition {
    TopLeft = 1,
    TopRight = 2,
    BottomLeft = 3,
    BottomRight = 4,
};

enum class DragMode {
    Sync = 1,
    Defered = 2,
};

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
    GMESSAGE_AUTO("abstract");
};
JSON_AUTO(GMessage, type)

struct HeartBeat : public GMessage
{
    std::string type = "heartbeat";
    virtual std::string msgType() const { return type; }
};

JSON_AUTO(HeartBeat, type)

// from main to game
struct Window : public GMessage
{
    GMESSAGE_AUTO("window");

    std::uint32_t windowId;
    std::uint32_t nativeHandle;
    std::string name;
    bool transparent = false;
    bool resizable = false;
    std::uint32_t maxWidth = 0;
    std::uint32_t maxHeight = 0;
    std::uint32_t minWidth = 0;
    std::uint32_t minHeight = 0;
    std::uint32_t dragBorderWidth = 0;

    std::string bufferName;
    WindowRect rect;
    std::optional<WindowCaptionMargin> caption;
};

JSON_AUTO(Window, type, windowId, nativeHandle, name, transparent, resizable, maxWidth, maxHeight, minWidth, minHeight, dragBorderWidth, bufferName, rect, caption)

struct WindowClose : public GMessage
{
    GMESSAGE_AUTO("window.close");

    std::uint32_t windowId;
};
JSON_AUTO(WindowClose, type, windowId)

struct WindowBounds : public GMessage
{
    GMESSAGE_AUTO("window.bounds");

    std::uint32_t windowId = 0;
    WindowRect rect;
    std::optional<std::string> bufferName;
};
JSON_AUTO(WindowBounds, type, windowId, rect, bufferName)

struct WindowFrameBuffer : public GMessage
{
    GMESSAGE_AUTO("window.framebuffer");

    std::uint32_t windowId = 0;
};

JSON_AUTO(WindowFrameBuffer, type, windowId)

struct CursorCommand : public GMessage
{
    GMESSAGE_AUTO("command.cursor");
    std::string cursor;
};

JSON_AUTO(CursorCommand, type, cursor)


struct FpsCommand : public GMessage
{
    GMESSAGE_AUTO("command.fps");

    bool showfps = false;
    std::uint32_t position = 1;
};

JSON_AUTO(FpsCommand, type, showfps, position)

struct OverlayInit : public GMessage
{
    GMESSAGE_AUTO("overlay.init");

    bool processEnabled;
    std::string shareMemMutex;

    std::vector<Hotkey> hotkeys;
    std::vector<Window> windows;

    bool showfps = false;
    std::uint32_t fpsPosition = 1;

    std::uint32_t dragMode = 1;
};

JSON_AUTO(OverlayInit, type, processEnabled, shareMemMutex, hotkeys, windows, showfps, fpsPosition, dragMode)

struct OverlayEnable : public GMessage
{
    GMESSAGE_AUTO("overlay.enable");

    bool processEnabled;
};

JSON_AUTO(OverlayEnable, type, processEnabled)

struct HotkeyInfo : public GMessage
{
    GMESSAGE_AUTO("overlay.hotkey");

    std::vector<Hotkey> hotkeys;
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
    GMESSAGE_AUTO("game.process");

    std::string path;
};

JSON_AUTO(GameProcessInfo, type, path)

struct GameExit : public GMessage
{
    GMESSAGE_AUTO("game.exit");
};

JSON_AUTO(GameExit, type)

struct InputHookInfo : public GMessage
{
    GMESSAGE_AUTO("input.hook");

    bool hooked = false;
};
JSON_AUTO(InputHookInfo, type, hooked)

struct GraphicsHookInfo : public GMessage
{
    GMESSAGE_AUTO("graphics.hook");

    std::string graphics;
    std::optional<D3d9HookInfo> d3d9hookInfo;
    std::optional<DxgiHookInfo> dxgihookInfo;
};

inline void from_json(const json &j, GraphicsHookInfo &object)
{
    object.graphics = j["graphics"].get<std::string>();

    if (object.graphics == "d3d9")
    {
        object.d3d9hookInfo = D3d9HookInfo(j["hookInfo"]);
    }
    else if (object.graphics == "dxgi")
    {
        object.dxgihookInfo = DxgiHookInfo(j["hookInfo"]);
    }
}

inline void to_json(json &j, const GraphicsHookInfo &object)
{
    j = {
        { "type", object.type },
        { "graphics", object.graphics },
    };

    if (object.graphics == "d3d9")
    {
        j["hookInfo"] = object.d3d9hookInfo.value();
    }
    else if (object.graphics == "dxgi")
    {
        j["hookInfo"] = object.dxgihookInfo.value();
    }
}

struct GraphicsWindowSetup : public GMessage
{
    GMESSAGE_AUTO("graphics.window");

    std::uint32_t window;
    int width;
    int height;
    bool focus;
    bool hooked;
};

JSON_AUTO(GraphicsWindowSetup, type, window, width, height, focus, hooked)

struct GraphicsWindowFocusEvent : public GMessage
{
    GMESSAGE_AUTO("graphics.window.event.focus");

    std::uint32_t window;
    bool focus;
};

JSON_AUTO(GraphicsWindowFocusEvent, type, window, focus)

struct GraphicsWindowRezizeEvent : public GMessage
{
    GMESSAGE_AUTO("graphics.window.event.resize");

    std::uint32_t window;
    int width;
    int height;
};

JSON_AUTO(GraphicsWindowRezizeEvent, type, window, width, height)

struct GraphicsWindowDestroyEvent : public GMessage
{
    GMESSAGE_AUTO("graphics.window.event.destroy");

    std::uint32_t window;
};

JSON_AUTO(GraphicsWindowDestroyEvent, type, window)


struct GraphicsFps : public GMessage
{
    GMESSAGE_AUTO("graphics.fps");

    std::uint32_t fps;
};

JSON_AUTO(GraphicsFps, type, fps)

struct GameInputIntercept : public GMessage
{
    GMESSAGE_AUTO("game.input.intercept");

    bool intercepting;
};

JSON_AUTO(GameInputIntercept, type, intercepting)

struct GameInput : public GMessage
{
    GMESSAGE_AUTO("game.input");

    std::uint32_t windowId;
    std::uint32_t msg;
    std::uint32_t wparam;
    std::uint32_t lparam;
};

JSON_AUTO(GameInput, type, windowId, msg, wparam, lparam)

} // namespace overlay
