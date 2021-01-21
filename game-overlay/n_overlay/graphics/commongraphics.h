#pragma once

static const char k_overlayIWindow[] = "n_overlay_win_x0y1x2";

struct CommonWindowSprite
{
    int windowId;
    std::string name;
    overlay::WindowRect rect;
    std::string bufferName;
    bool alwaysOnTop;

    std::unique_ptr<windows_shared_memory> windowBitmapMem;
};

class CommonGraphics : public Storm::Trackable<>
{
protected:
    std::mutex synclock_;
    struct SyncState
    {
        std::set<std::uint32_t> pendingWindows_;
        std::set<std::uint32_t> pendingFrameBuffers_;
        std::set<std::uint32_t> pendingClosed_;
        std::map<std::uint32_t, overlay::WindowRect> pendingBounds_;
        std::set<std::uint32_t> pendingFrameBufferUpdates_;
        std::uint32_t focusWindowId_ = 0;
    } syncState_;

    std::atomic<bool> needResync_ = false;

    bool windowed_ = false;

    std::vector<std::shared_ptr<CommonWindowSprite>> windowSprites_;

public:

    virtual bool isWindowed() const;
        
    virtual bool _createSprites() { return false; };
    virtual void _createWindowSprites();

    virtual std::shared_ptr<CommonWindowSprite> _createWindowSprite(const std::shared_ptr<overlay::Window>& window);
    virtual void _updateSprite(std::shared_ptr<CommonWindowSprite>& sprite, bool clear = false) = 0;

    virtual void _syncPendingBounds(std::map<std::uint32_t, overlay::WindowRect> pendingBounds_) = 0;

    virtual void _checkAndResyncWindows();

    virtual void _drawBlockSprite() = 0;
    virtual void _drawWindowSprites();
    virtual void _drawWindowSprite(std::shared_ptr<CommonWindowSprite>&) = 0;
};