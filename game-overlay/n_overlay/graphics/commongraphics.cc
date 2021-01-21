#include "stable.h"

#include "overlay/hookapp.h"
#include "overlay/overlay.h"
#include "overlay/session.h"

bool CommonGraphics::isWindowed() const
{
    return windowed_;
}
void CommonGraphics::_createWindowSprites()
{
    HookApp::instance()->overlayConnector()->lockWindows();

    auto windows = HookApp::instance()->overlayConnector()->windows();
    for (const auto& w : windows)
    {
        auto windowSprite = _createWindowSprite(w);
        if (windowSprite)
        {
            windowSprites_.push_back(windowSprite);
        }
    }

    HookApp::instance()->overlayConnector()->unlockWindows();
}
std::shared_ptr<CommonWindowSprite> CommonGraphics::_createWindowSprite(const std::shared_ptr<overlay::Window>& window)
{
    std::shared_ptr<CommonWindowSprite> windowSprite(new CommonWindowSprite);
    windowSprite->windowId = window->windowId;
    windowSprite->name = window->name;
    windowSprite->bufferName = window->bufferName;
    windowSprite->rect = window->rect;
    windowSprite->alwaysOnTop = window->alwaysOnTop;

    try
    {
        windows_shared_memory share_mem(windows_shared_memory::open_only, windowSprite->bufferName.c_str(), windows_shared_memory::read_only);
        windowSprite->windowBitmapMem = std::make_unique<windows_shared_memory>(std::move(share_mem));
    }
    catch (...)
    {
        return nullptr;
    }

    return windowSprite;
}
void CommonGraphics::_checkAndResyncWindows()
{
    if (needResync_)
    {
        SyncState syncState;
        {
            std::lock_guard<std::mutex> lock(synclock_);
            syncState.pendingWindows_.swap(syncState_.pendingWindows_);
            syncState.pendingFrameBuffers_.swap(syncState_.pendingFrameBuffers_);
            syncState.pendingClosed_.swap(syncState_.pendingClosed_);
            syncState.pendingBounds_.swap(syncState_.pendingBounds_);
            syncState.pendingFrameBufferUpdates_.swap(syncState_.pendingFrameBufferUpdates_);
            syncState.focusWindowId_ = syncState_.focusWindowId_;
        }
        if (syncState.pendingWindows_.size() > 0 || syncState.pendingFrameBufferUpdates_.size() > 0)
        {
            HookApp::instance()->overlayConnector()->lockWindows();

            auto windows = HookApp::instance()->overlayConnector()->windows();

            for (auto windowId : syncState.pendingWindows_)
            {
                auto it = std::find_if(windows.begin(), windows.end(), [windowId](const auto& window) {
                    return windowId == window->windowId;
                });
                if (it != windows.end())
                {
                    if (auto windowSprite = _createWindowSprite(*it))
                    {
                        windowSprites_.push_back(windowSprite);
                    }
                }
            }

            for (auto windowId : syncState.pendingFrameBufferUpdates_)
            {
                auto it = std::find_if(windowSprites_.begin(), windowSprites_.end(), [windowId](const auto& window) {
                    return windowId == window->windowId;
                });
                if (it != windowSprites_.end())
                {
                    auto& windowSprite = *it;
                    try
                    {
                        windows_shared_memory share_mem(windows_shared_memory::open_only, windowSprite->bufferName.c_str(), windows_shared_memory::read_only);
                        windowSprite->windowBitmapMem = std::make_unique<windows_shared_memory>(std::move(share_mem));
                    }
                    catch (...)
                    {
                    }
                }
            }

            HookApp::instance()->overlayConnector()->unlockWindows();
        }

        if (syncState.pendingClosed_.size() > 0)
        {
            for (auto windowId : syncState.pendingClosed_)
            {
                auto it = std::find_if(windowSprites_.begin(), windowSprites_.end(), [windowId](const auto& window) {
                    return windowId == window->windowId;
                });
                if (it != windowSprites_.end())
                {
                    windowSprites_.erase(it);
                }
            }
        }

        if (syncState.pendingBounds_.size() > 0)
        {
            _syncState(syncState.pendingBounds_);
        }

        if (syncState.pendingFrameBuffers_.size() > 0)
        {
            for (auto windowId : syncState.pendingFrameBuffers_)
            {
                auto it = std::find_if(windowSprites_.begin(), windowSprites_.end(), [windowId](const auto& window) {
                    return windowId == window->windowId;
                });

                if (it != windowSprites_.end())
                {
                    _updateSprite(*it);
                }
            }
        }

        if (syncState.focusWindowId_)
        {
            if (windowSprites_.at(windowSprites_.size() - 1)->windowId != syncState.focusWindowId_)
            {
                auto it = std::find_if(windowSprites_.begin(), windowSprites_.end(), [&](const auto& w) {
                    return w->windowId == syncState.focusWindowId_;
                });
                if (it != windowSprites_.end())
                {
                    auto focusWindow = *it;
                    windowSprites_.erase(it);
                    windowSprites_.push_back(focusWindow);
                }
            }
        }

        needResync_ = false;
    }
}
void CommonGraphics::_drawWindowSprites()
{
    for (auto& windowSprite : windowSprites_)
    {
        if (!windowSprite->alwaysOnTop && !HookApp::instance()->uiapp()->isInterceptingInput())
            continue;
        _drawWindowSprite(windowSprite);
    }
}