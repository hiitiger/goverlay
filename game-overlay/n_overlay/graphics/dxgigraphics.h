
#pragma once

class DxgiGraphics : public Storm::Trackable<>
{
protected:
    std::mutex synclock_;
    std::set<std::uint32_t> pendingWindows_;
    std::set<std::uint32_t> pendingFrameBuffers_;
    std::set<std::uint32_t> pendingClosed_;
    std::map<std::uint32_t, overlay::WindowRect> pendingBounds_;

    std::atomic<bool> needResync_ = false;
    FpsTimer fpsTimer_;

public:
    virtual ~DxgiGraphics() {}

    virtual Windows::ComPtr<IDXGISwapChain> swapChain() const = 0;

    virtual bool initGraphics(IDXGISwapChain *swap);

    virtual void uninitGraphics(IDXGISwapChain *swap);
    virtual void freeGraphics();

    virtual void beforePresent(IDXGISwapChain *swap);
    virtual void afterPresent(IDXGISwapChain *swap) ;

    virtual bool _initGraphicsContext(IDXGISwapChain* swap) = 0;
    virtual bool _initGraphicsState() = 0;
    virtual void _initSpriteDrawer() = 0;
    virtual void _createSprites() = 0;
    virtual void _createWindowSprites() = 0;

    virtual void _checkAndResyncWindows() = 0;

    virtual void _drawBlockSprite() = 0;
    virtual void _drawWindowSprites() = 0;
    virtual void _drawMainSprite() = 0;

    virtual void _saveStatus() = 0;
    virtual void _prepareStatus() = 0;
    virtual void _restoreStatus() = 0;

};
