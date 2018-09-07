#include <assert.h>

#include "boost/optional.hpp"
#include "nlohmann/json.hpp"
#include "ipc/ipcmsg.h"

//just use json
//whatever...

using json = nlohmann::json;

#ifdef _MSC_VER // Microsoft compilers

#define GET_ARG_COUNT(...) INTERNAL_EXPAND_ARGS_PRIVATE(INTERNAL_ARGS_AUGMENTER(__VA_ARGS__))

#define INTERNAL_ARGS_AUGMENTER(...) unused, __VA_ARGS__
#define INTERNAL_EXPAND(x) x
#define INTERNAL_EXPAND_ARGS_PRIVATE(...) INTERNAL_EXPAND(INTERNAL_GET_ARG_COUNT_PRIVATE(__VA_ARGS__, 69, 68, 67, 66, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))
#define INTERNAL_GET_ARG_COUNT_PRIVATE(_1_, _2_, _3_, _4_, _5_, _6_, _7_, _8_, _9_, _10_, _11_, _12_, _13_, _14_, _15_, _16_, _17_, _18_, _19_, _20_, _21_, _22_, _23_, _24_, _25_, _26_, _27_, _28_, _29_, _30_, _31_, _32_, _33_, _34_, _35_, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, count, ...) count

#else // Non-Microsoft compilers

#define GET_ARG_COUNT(...) INTERNAL_GET_ARG_COUNT_PRIVATE(0, ##__VA_ARGS__, 70, 69, 68, 67, 66, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define INTERNAL_GET_ARG_COUNT_PRIVATE(_0, _1_, _2_, _3_, _4_, _5_, _6_, _7_, _8_, _9_, _10_, _11_, _12_, _13_, _14_, _15_, _16_, _17_, _18_, _19_, _20_, _21_, _22_, _23_, _24_, _25_, _26_, _27_, _28_, _29_, _30_, _31_, _32_, _33_, _34_, _35_, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, count, ...) count

#endif

#define VA_ARGS(...) , ##__VA_ARGS__

#define STRINGIZE(arg) STRINGIZE1(arg)
#define STRINGIZE1(arg) STRINGIZE2(arg)
#define STRINGIZE2(arg) #arg

#define CONCATENATE(arg1, arg2) CONCATENATE1(arg1, arg2)
#define CONCATENATE1(arg1, arg2) CONCATENATE2(arg1, arg2)
#define CONCATENATE2(arg1, arg2) arg1##arg2

#define FOR_EACH_0(what)
#define FOR_EACH_1(what, x1) what(x1)
#define FOR_EACH_2(what, x1, x2) \
    what(x1);                    \
    FOR_EACH_1(what, x2)
#define FOR_EACH_3(what, x1, x2, x3) \
    what(x1);                        \
    FOR_EACH_2(what, x2, x3)
#define FOR_EACH_4(what, x1, x2, x3, x4) \
    what(x1);                            \
    FOR_EACH_3(what, x2, x3, x4)
#define FOR_EACH_5(what, x1, x2, x3, x4, x5) \
    what(x1);                                \
    FOR_EACH_4(what, x2, x3, x4, x5)
#define FOR_EACH_6(what, x1, x2, x3, x4, x5, x6) \
    what(x1);                                    \
    FOR_EACH_5(what, x2, x3, x4, x5, x6)
#define FOR_EACH_7(what, x1, x2, x3, x4, x5, x6, x7) \
    what(x1);                                        \
    FOR_EACH_6(what, x2, x3, x4, x5, x6, x7)
#define FOR_EACH_8(what, x1, x2, x3, x4, x5, x6, x7, x8) \
    what(x1);                                            \
    FOR_EACH_7(what, x2, x3, x4, x5, x6, x7, x8)

#define FOR_EACH_(N, what, ...) CONCATENATE(FOR_EACH_, N)(what VA_ARGS(__VA_ARGS__))
#define FOR_EACH(what, ...) FOR_EACH_(GET_ARG_COUNT(__VA_ARGS__), what VA_ARGS(__VA_ARGS__))

template <class A>
struct json_setter
{
    static void set(const A &field, json &j, const char *str)
    {
        j[str] = field;
    }
};

template <class A>
struct json_setter<boost::optional<A>>
{
    static void set(const boost::optional<A> &field, json &j, const char *str)
    {
        if (field)
        {
            j[str] = field.value();
        }
    }
};

template <class A>
struct json_getter
{
    static void get(A &field, const json &j, const char *str)
    {
        field = j.at(str).get<A>();
    }
};

template <class A>
struct json_getter<boost::optional<A>>
{
    static void get(boost::optional<A> &field, const json &j, const char *str)
    {
        if (j.find(str) != j.cend())
        {
            field = j[str].get<A>();
        }
    }
};

#define JSON_SET(field) \
    json_setter<std::decay<decltype(object.field)>::type>::set(object.field, j, STRINGIZE(field))

#define JSON_GET(field) \
    json_getter<std::decay<decltype(object.field)>::type>::get(object.field, j, STRINGIZE(field))

#define JSON_SERIALIZE(CLS, ...)                    \
    inline void to_json(json &j, const CLS &object) \
    {                                               \
        j = json{};                                 \
        FOR_EACH(JSON_SET, __VA_ARGS__);            \
    }

#define JSON_DESERIALIZE(CLS, ...)                    \
    inline void from_json(const json &j, CLS &object) \
    {                                                 \
        FOR_EACH(JSON_GET, __VA_ARGS__);              \
    }

#define JSON_AUTO(CLS, ...)            \
    JSON_DESERIALIZE(CLS, __VA_ARGS__) \
    JSON_SERIALIZE(CLS, __VA_ARGS__)

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
    virtual bool fromJson(const json &obj) = 0;
    virtual json toJson(bool *ok = false) const = 0;
};

struct HeartBeat : public GMessage
{
    std::string type = "heartbeat";

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

    std::uint32_t windowId;
    std::string name;
    bool transparent = false;
    bool resizable = false;

    std::string bufferName;

    WindowRect rect;
    boost::optional<WindowCaptionMargin> caption;

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
    std::string cursor;
};

JSON_AUTO(CursorCommand, type, cursor)

struct FpsCommand : public GMessage
{
    std::string type = "command.fps";

    bool showfps;
    int position;
};

JSON_AUTO(FpsCommand, type, showfps, position)

struct OverlayInit : public GMessage
{
    std::string type = "overlay.init";

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
};

} // namespace overlay
