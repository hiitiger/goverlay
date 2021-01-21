#include "stable.h"
#include "overlay/hookapp.h"
#include "overlay/session.h"
#include "dxgigraphics.h"

bool DxgiGraphics::isWindowed() const
{
    return windowed_;
}

bool DxgiGraphics::initGraphics(IDXGISwapChain *swap)
{
    bool succeed = _initGraphicsContext(swap) && _initGraphicsState();

    if (succeed)
    {
        _initSpriteDrawer();
        _createSprites();
        _createWindowSprites();
    }

    if (!succeed)
    {
        freeGraphics();
    }

    if (succeed)
    {
        HookApp::instance()->overlayConnector()->windowEvent().add([this](std::uint32_t windowId) {
            std::lock_guard<std::mutex> lock(synclock_);
            syncState_.pendingWindows_.insert(windowId);
            needResync_ = true;
        }, this);

        HookApp::instance()->overlayConnector()->frameBufferEvent().add([this](std::uint32_t windowId) {
            std::lock_guard<std::mutex> lock(synclock_);
            syncState_.pendingFrameBuffers_.insert(windowId);
            needResync_ = true;
        }, this);

        HookApp::instance()->overlayConnector()->windowCloseEvent().add([this](std::uint32_t windowId) {
            std::lock_guard<std::mutex> lock(synclock_);
            syncState_.pendingClosed_.insert(windowId);
            needResync_ = true;
        }, this);

        HookApp::instance()->overlayConnector()->windowBoundsEvent().add([this](std::uint32_t windowId, overlay::WindowRect rect) {
            std::lock_guard<std::mutex> lock(synclock_);
            syncState_.pendingBounds_[windowId] = rect;
            needResync_ = true;
        }, this);

        HookApp::instance()->overlayConnector()->frameBufferUpdateEvent().add([this](std::uint32_t windowId) {
            std::lock_guard<std::mutex> lock(synclock_);
            syncState_.pendingFrameBufferUpdates_.insert(windowId);
            needResync_ = true;
        }, this);

        HookApp::instance()->overlayConnector()->windowFocusEvent().add([this](std::uint32_t windowId) {
            std::lock_guard<std::mutex> lock(synclock_);
            syncState_.focusWindowId_ = windowId;
            needResync_ = true;
        }, this);
    }

    if (succeed)
    {
        fpsTimer_.reset();
    }

    return succeed;
}

void DxgiGraphics::uninitGraphics(IDXGISwapChain *swap)
{
    if (swapChain().get() != swap)
    {
        return;
    }

    freeGraphics();
}

void DxgiGraphics::freeGraphics()
{
    HookApp::instance()->overlayConnector()->windowEvent().remove(this);
    HookApp::instance()->overlayConnector()->frameBufferEvent().remove(this);
    HookApp::instance()->overlayConnector()->windowCloseEvent().remove(this);
    HookApp::instance()->overlayConnector()->windowBoundsEvent().remove(this);
    HookApp::instance()->overlayConnector()->frameBufferUpdateEvent().remove(this);
    HookApp::instance()->overlayConnector()->windowFocusEvent().remove(this);

    std::lock_guard<std::mutex> lock(synclock_);
    syncState_.pendingWindows_.clear();
    syncState_.pendingFrameBuffers_.clear();
    syncState_.pendingClosed_.clear();
    syncState_.pendingBounds_.clear();
    syncState_.pendingFrameBufferUpdates_.clear();
    syncState_.focusWindowId_ = 0;
    needResync_ = false;
}

void DxgiGraphics::beforePresent(IDXGISwapChain *swap)
{
    if (swapChain().get() != swap)
    {
        return;
    }

    session::setGraphicsThreadId(GetCurrentThreadId());

    if (fpsTimer_.tick() > 1000.)
    {
        HookApp::instance()->overlayConnector()->sendGraphicsFps(fpsTimer_.fps());
    }

    _saveStatus();
    _prepareStatus();

    _checkAndResyncWindows();

    if (HookApp::instance()->uiapp()->isInterceptingInput())
    {
        _drawBlockSprite();

    }

    _drawWindowSprites();
    /*
#if 0
    _drawMainSprite();
#endif

    _drawPopupTipSprite();
    _drawStatutBarSprite();
    */
    _restoreStatus();
}

void DxgiGraphics::afterPresent(IDXGISwapChain *swap)
{
    if (swapChain().get() != swap)
    {
        return;
    }
}
