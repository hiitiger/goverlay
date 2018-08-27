#include <assert.h>

#include "boost/optional.hpp"
#include "nlohmann/json.hpp"
#include "ipc/ipcmsg.h"

//just use json
//whatever...

namespace overlay
{

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

    Hotkey(const json &obj)
    {
        vKey = obj["vKey"].get<int>();
        modifiers = obj["modifiers"].get<int>();
        passthrough = obj["passthrough"].get<bool>();
        name = obj["name"].get<std::string>();
    }

    json toJson()
    {
        return {
            {"vKey", vKey},
            {"modifiers", vKey},
            {"passthrough", passthrough},
            {"name", name},
        };
    }
};

struct WindowRect
{
    int x;
    int y;
    int width;
    int height;

    WindowRect() = default;

    WindowRect(const json &obj)
    {
        x = obj["x"].get<int>();
        y = obj["y"].get<int>();
        width = obj["width"].get<int>();
        height = obj["height"].get<int>();
    }

    json toJson()
    {
        return {
            {"x", x},
            {"y", y},
            {"width", width},
            {"height", height},
        };
    }
};

struct WindowCaptionMargin
{
    int left;
    int right;
    int top;
    int height;

    WindowCaptionMargin() = default;

    WindowCaptionMargin(const json &obj)
    {
        left = obj["left"].get<int>();
        right = obj["right"].get<int>();
        top = obj["top"].get<int>();
        height = obj["height"].get<int>();
    }

    json toJson()
    {
        return {
            {"left", left},
            {"right", right},
            {"top", top},
            {"height", height},
        };
    }
};

struct GMessage
{
    std::string type = "abstract";
    virtual bool fromJson(const json &obj) = 0;
    virtual bool toJson(json &obj) = 0;
};

struct HeartBeat : public GMessage
{
    std::string type = "heartbeat";

    virtual bool fromJson(const json &obj)
    {
        assert(obj["type"].get<std::string>() == this->type);
        return true;
    }

    virtual bool toJson(json &obj)
    {
        obj = {{"type", type}};
        return true;
    }
};

// from main to game
struct Window : public GMessage
{
    std::string type = "window";

    std::uint32_t windowId;
    std::string name;
    bool resizable = false;

    std::string bufferName;

    boost::optional<WindowRect> rect;
    boost::optional<WindowCaptionMargin> caption;

    virtual bool fromJson(const json &obj)
    {
        assert(obj["type"].get<std::string>() == this->type);
        this->name = obj["name"].get<std::string>();
        this->windowId = static_cast<std::uint32_t>(obj["windowId"].get<std::uint32_t>());
        this->resizable = obj["resizable"].get<bool>();
        this->bufferName = obj["bufferName"].get<std::string>();

        if (obj["rect"].is_object())
        {
            this->rect = WindowRect(obj["rect"]);
        }
        if (obj["caption"].is_object())
        {
            this->caption = WindowCaptionMargin(obj["caption"]);
        }
    }
    virtual bool toJson(json &obj)
    {
        obj = {
            {"type", this->type},
            {"windowId", this->windowId},
            {"resizable", this->resizable},
            {"bufferName", this->bufferName}};

        if (this->rect)
        {
            obj["rect"] = this->rect->toJson();
        }
        if (this->caption) 
        {
            obj["caption"] = this->caption->toJson();
        }

        return true;
    }
};

struct FrameBuffer : public GMessage
{
    std::string type = "window.framebuffer";

    std::uint32_t windowId;
    std::string bufferName;

    virtual bool fromJson(const json &obj)
    {
        assert(obj["type"].get<std::string>() == this->type);
        this->windowId = obj["windowId"].get<std::uint32_t>();
        this->bufferName = obj["bufferName"].get<std::string>();
        return true;
    }
    virtual bool toJson(json &obj)
    {
        obj = {
            {"type", type},
            {"windowId", windowId},
            {"bufferName", bufferName},
        };

        return true;
    }
};

struct CursorCommand : public GMessage
{
    std::string type = "command.cursor";
    std::string cursor;
};

struct FpsCommand : public GMessage
{
    std::string type = "command.fps";

    bool showfps;
    int position;
};

struct OverlayInit : public GMessage
{
    std::string type = "overlay.init";

    std::vector<Hotkey> hotkeys;

    virtual bool fromJson(const json &obj)
    {
        assert(obj["type"].get<std::string>() == this->type);
        assert(obj["hotkeys"].is_array());
        json hotkeyArr = obj["hotkeys"];
        for (auto i = 0; i != hotkeyArr.size(); i++)
        {
            this->hotkeys.emplace_back(hotkeyArr[i]);
        }
        return true;
    }

    virtual bool toJson(json &obj)
    {
        json arr;
        for (auto hotkey : this->hotkeys)
        {
            arr.push_back(hotkey.toJson());
        }

        obj = arr;
        return true;
    }
};

struct HotkeyInfo : public GMessage
{
    std::string type = "overlay.hotkey";

    std::vector<Hotkey> hotkeys;

    virtual bool fromJson(const json &obj)
    {
        assert(obj["type"].get<std::string>() == this->type);
        assert(obj["hotkeys"].is_array());
        json hotkeyArr = obj["hotkeys"];
        for (auto i = 0; i != hotkeyArr.size(); i++)
        {
            this->hotkeys.emplace_back(hotkeyArr[i]);
        }
        return true;
    }

    virtual bool toJson(json &obj)
    {
        json arr;
        for (auto hotkey : this->hotkeys)
        {
            arr.push_back(hotkey.toJson());
        }

        obj = arr;
        return true;
    }
};

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

    D3d9HookInfo(const json &obj)
    {
        endSceneHooked = obj["endSceneHooked"].get<bool>();
        presentHooked = obj["presentHooked"].get<bool>();
        presentExHooked = obj["presentExHooked"].get<bool>();
        swapChainPresentHooked = obj["swapChainPresentHooked"].get<bool>();
        resetHooked = obj["resetHooked"].get<bool>();
        resetExHooked = obj["resetExHooked"].get<bool>();
    }

    json toJson()
    {
        return {
            {"endSceneHooked", endSceneHooked},
            {"presentHooked", presentHooked},
            {"presentExHooked", presentExHooked},
            {"swapChainPresentHooked", swapChainPresentHooked},
            {"resetHooked", resetHooked},
            {"resetExHooked", resetExHooked},
        };
    }
};

struct DxgiHookInfo
{
    DxgiHookInfo() = default;

    DxgiHookInfo(const json &obj)
    {
    }

    json toJson()
    {
        return {};
    }
};

struct MouseInput
{
};

struct KeyInput
{
};

struct GameProcessInfo : public GMessage
{
    std::string type = "game.process";
    std::string path;

    virtual bool fromJson(const json &obj)
    {
        assert(obj["type"].get<std::string>() == this->type);
        this->path = obj["path"].get<std::string>();
        return true;
    }

    virtual bool toJson(json &obj)
    {
        obj = {
            {"type", type},
            {"path", path}};
        return true;
    }
};

struct OverlayExit : public GMessage
{
    std::string type = "game.exit";
    virtual bool fromJson(const json &obj)
    {
        assert(obj["type"].get<std::string>() == this->type);
        return true;
    }

    virtual bool toJson(json &obj)
    {
        obj = {
            {"type", type}};
        return true;
    }
};

struct GraphicsHookInfo : public GMessage
{
    std::string type = "graphics.hook";
    std::string graphics;
    boost::optional<D3d9HookInfo> d3d9hookInfo;
    boost::optional<DxgiHookInfo> dxgihookInfo;

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

    virtual bool toJson(json &obj)
    {
        obj = {
            {"type", type},
            {"graphics", graphics},
            };

        if (this->graphics == "d3d9") 
        {
            obj["hookInfo"] = this->d3d9hookInfo->toJson();
        }
        else if (this->graphics == "dxgi") 
        {
            obj["hookInfo"] = this->dxgihookInfo->toJson();
        }
        return true;
    }
};

struct GraphicsWindowSetup : public GMessage
{
    std::string type = "graphics.window";
};

struct GraphcisWindowFocusEvent : public GMessage
{
    std::string type = "graphics.window.event.focus";
};

struct GraphcisWindowRezizeEvent : public GMessage
{
    std::string type = "graphics.window.event.resize";
};

struct GameInputIntercept : public GMessage
{
    std::string type = "game.input.intercept";
};

struct GameInput : public GMessage
{
    std::string type = "game.input";
};

} // namespace overlay
