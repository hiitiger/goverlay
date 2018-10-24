#pragma once

struct ID3DXSprite;


struct D3d9WindowSprite {
    int windowId;
    std::string name;
    overlay::WindowRect rect;
    std::string bufferName;

    std::unique_ptr<windows_shared_memory> windowBitmapMem;
    Windows::ComPtr<IDirect3DTexture9> texture;
};


class D3d9Graphics : public Storm::Trackable<>
{
    std::mutex synclock_;
    std::set<std::uint32_t> pendingWindows_;
    std::set<std::uint32_t> pendingFrameBuffers_;
    std::set<std::uint32_t> pendingClosed_;
    std::map<std::uint32_t, overlay::WindowRect> pendingBounds_;
    std::set<std::uint32_t> pendingFrameBufferUpdates_;
    std::uint32_t focusWindowId_ = 0;

    std::atomic<bool> needResync_ = false;
    FpsTimer fpsTimer_;

    Windows::ComPtr<IDirect3DDevice9> device_;

    bool windowed_ = false;

    int targetWidth_ = 0;
    int targetHeight_ = 0;
    D3DFORMAT d3dformat_ = D3DFMT_UNKNOWN;

    Windows::ComPtr<IDirect3DTexture9> blockSprite_;
    Windows::ComPtr<ID3DXSprite> spriteDrawer_;


    std::vector<std::shared_ptr<D3d9WindowSprite>> windowSprites_;
    std::shared_ptr<D3d9WindowSprite> mainSprite_;
    std::shared_ptr<D3d9WindowSprite> statusBarSprite_;
    std::shared_ptr<D3d9WindowSprite> overlayTipSprite_;
public:
    D3d9Graphics();
    ~D3d9Graphics();

    bool isWindowed() const;

    bool initGraphics(IDirect3DDevice9* device, HWND hDestWindowOverride, bool isD9Ex);
    void uninitGraphics(IDirect3DDevice9* device);
    void freeGraphics();

    void beforePresent(IDirect3DDevice9* device);
    void afterPresent(IDirect3DDevice9* device);

    bool _initGraphicsContext(IDirect3DDevice9* device);

    bool _initSpriteDrawer();

    bool _createSprites();
    void _createWindowSprites();

    Windows::ComPtr<IDirect3DTexture9> _createDynamicTexture(std::uint32_t width, std::uint32_t height);

    std::shared_ptr<D3d9WindowSprite > _createWindowSprite(const std::shared_ptr<overlay::Window>& window);

    void _updateSprite(std::shared_ptr<D3d9WindowSprite>& sprite, bool clear = false);

    void _checkAndResyncWindows() ;

    void _drawBlockSprite() ;
    void _drawWindowSprites() ;
    void _drawMainSprite() ;
    void _drawStatutBarSprite() ;
    void _drawPopupTipSprite() ;

    void _drawWindowSprite(std::shared_ptr<D3d9WindowSprite>&);
};