#pragma once

#include "overlay.h"
#include "uiapp.h"

struct IGraphicsHook;
class D3d9Hook;
class DXGIHook;

class HookApp
{
    std::wstring processPath_;
    std::wstring processName_;

    std::atomic<bool> hookFlag_ = false;
    std::atomic<bool> quitFlag_ = false;

    std::mutex runloopLock_;
    std::unique_ptr<Storm::CoreRunloop> runloop_;

    Storm::WaitableEvent hookQuitedEvent_;

    std::shared_ptr<OverlayConnector> overlay_;
    std::shared_ptr<UiApp> uiapp_;

    HANDLE hookloopThread_ = nullptr;

  public:
    HookApp();
    ~HookApp();

    static void initialize();
    static void uninitialize();

    static HookApp *instance();

    std::wstring procPath() const { return processPath_;}
    std::wstring procName() const { return processName_;}

    bool isQuitSet() const { return quitFlag_; }

    std::shared_ptr<OverlayConnector> overlayConnector() const { return overlay_; }

    std::shared_ptr<UiApp> uiapp() const { return uiapp_;}

    HANDLE start();

    void quit();

    void startHook();

    void async(const std::function<void()>& task);

    void deferHook();

    void hookThread();

private:

    bool findGameWindow();

    void hook();

    bool hookWindow();

    void unhookGraphics();
    void hookGraphics();

    bool hookInput();

    bool hookD3d9();
    bool hookDXGI();

};