#pragma once
#include "hook/inputhook.h"
#include "overlay/hookapp.h"
#include "overlay/uiapp.h"



class HotkeyCheck : public Storm::Trackable<>
{
    struct HotKey
    {
        std::string name;
        std::vector<std::pair<std::int16_t, bool>> keys;
        bool wasDown = false;
    };

    std::mutex mutex;
    std::thread worker;
    std::map<std::string, HotKey> hotkeys;

    std::atomic<bool> shutdown = false;

public:
    static HotkeyCheck * instance();

    bool checkHotkeys();

    void start();
    void stop();
    void updateHotkeys(const std::vector<HotKey>&);

    void _onHotkeyDown(const std::string& name);
    void _onHotkeyUp(const std::string& name);

    static int32_t hotKeyThread(void* arg);
    void _hotkeyThread();
};

inline int32_t HotkeyCheck::hotKeyThread(void* arg)
{
    HotkeyCheck* td = static_cast<HotkeyCheck*>(arg);
    td->_hotkeyThread();
    return 0;
}

inline HotkeyCheck * HotkeyCheck::instance()
{
    static HotkeyCheck ins;
    return &ins;
}

inline void HotkeyCheck::start()
{
#ifdef HOTKEY_THREADED

    if (worker.joinable())
    {
        return;
    }

    worker = std::thread(hotKeyThread, this);

#endif

    HookApp::instance()->overlayConnector()->hotkeysEvent().add([this](std::vector<overlay::Hotkey> hotkeys) {
        std::vector<HotKey> transHotkeys;
        std::transform(hotkeys.begin(), hotkeys.end(), std::back_inserter(transHotkeys), [](const overlay::Hotkey& hotkey) {
            std::vector<std::pair<std::int16_t, bool>> keys = {
                { VK_SHIFT, hotkey.shift },
                { VK_CONTROL, hotkey.ctrl },
                { VK_MENU, hotkey.alt },
                { hotkey.keyCode, true }
            };

            return HotKey{ hotkey.name, keys };
        });

        this->updateHotkeys(transHotkeys);
    });
}

inline void HotkeyCheck::stop()
{
    shutdown = true;

#ifdef HOTKEY_THREADED
    if (!worker.joinable())
    {
        return;
    }
    worker.join();
#endif
}

inline void HotkeyCheck::updateHotkeys(const std::vector<HotKey>& hotkeys)
{
    std::unique_lock<std::mutex> ulock(mutex);
    this->hotkeys.clear();
    for (const auto& hotkey: hotkeys)
    {
        this->hotkeys.insert(std::make_pair(hotkey.name, hotkey));
    }
}

inline bool HotkeyCheck::checkHotkeys()
{
    bool checked = false;
    DWORD threadId = GetCurrentThreadId();
    std::unique_lock<std::mutex> ulock(mutex);

    for (auto&[name, hotkey] : hotkeys)
    {
        if (!shutdown)
        {
            bool allPressed = true;

            for (std::pair<std::int16_t, bool> k : hotkey.keys)
            {
                bool isBound = k.second;
                bool isPressed = Windows::OrginalApi::GetAsyncKeyState(k.first) & 0x8000;

                if ((isBound && !isPressed) || (!isBound && isPressed))
                    allPressed = false;
            }

            if (allPressed && !hotkey.wasDown)
            {
                if (threadId == session::windowThreadId())
                {
                    _onHotkeyDown(name);
                }
                else
                {
                    HookApp::instance()->uiapp()->async([this, name]() {
                        _onHotkeyDown(name);
                    });
                }
                hotkey.wasDown = true;
                checked = true;
            }
            else if (!allPressed && hotkey.wasDown)
            {
                if (threadId == session::windowThreadId())
                {
                    _onHotkeyUp(name);
                }
                else
                {
                    HookApp::instance()->uiapp()->async([this, name]() {
                        _onHotkeyUp(name);
                    });
                }
                hotkey.wasDown = false;
                checked = true;
            }
        }
    }
    return checked;
}

inline void HotkeyCheck::_hotkeyThread()
{
    while (!shutdown)
    {
        this->checkHotkeys();

        if (!shutdown)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}

inline void HotkeyCheck::_onHotkeyDown(const std::string& name)
{
    CHECK_THREAD(Threads::Window);

    if (session::overlayEnabled())
    {
        if (name == "overlay.toggle")
        {
            HookApp::instance()->uiapp()->toggleInputIntercept();
        }

        HookApp::instance()->overlayConnector()->sendInGameHotkeyDown(name);
    }
}

inline void HotkeyCheck::_onHotkeyUp(const std::string& )
{
    CHECK_THREAD(Threads::Window);

}

