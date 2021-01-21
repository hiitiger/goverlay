#pragma once

#include "..\commongraphics.h"

struct ID3DXSprite;

struct D3d9WindowSprite : CommonWindowSprite {
    Windows::ComPtr<IDirect3DTexture9> texture;
};


class D3d9Graphics : public CommonGraphics
{
    Windows::ComPtr<IDirect3DDevice9> device_;


    int targetWidth_ = 0;
    int targetHeight_ = 0;
    D3DFORMAT d3dformat_ = D3DFMT_UNKNOWN;

    Windows::ComPtr<IDirect3DTexture9> blockSprite_;
    Windows::ComPtr<ID3DXSprite> spriteDrawer_;

    FpsTimer fpsTimer_;
    
public:
    D3d9Graphics();
    ~D3d9Graphics();

    bool initGraphics(IDirect3DDevice9* device, HWND hDestWindowOverride, bool isD9Ex);
    void uninitGraphics(IDirect3DDevice9* device);
    void freeGraphics();

    void beforePresent(IDirect3DDevice9* device);
    void afterPresent(IDirect3DDevice9* device);

    bool _initGraphicsContext(IDirect3DDevice9* device);

    bool _initSpriteDrawer();

    bool _createSprites() override;
    void _createWindowSprites() override;

    Windows::ComPtr<IDirect3DTexture9> _createDynamicTexture(std::uint32_t width, std::uint32_t height);
    std::shared_ptr<CommonWindowSprite > _createWindowSprite(const std::shared_ptr<overlay::Window>& window) override;
    void _updateSprite(std::shared_ptr<CommonWindowSprite>& sprite, bool clear = false) override;

    void _syncPendingBounds(std::map<std::uint32_t, overlay::WindowRect> pendingBounds_) override;

    void _drawBlockSprite() override;
    void _drawWindowSprite(std::shared_ptr<CommonWindowSprite>&) override;
};
typedef IDirect3D9* (WINAPI* pFnDirect3DCreate9)(UINT SDKVersion);
typedef HRESULT(WINAPI* pFnDirect3DCreate9Ex)(UINT, void**);

typedef HRESULT(WINAPI* pFnCreateDXGIFactory1)(REFIID riid, void** ppFactory);

typedef HRESULT(STDMETHODCALLTYPE* PresentType)(IDirect3DDevice9* d, THIS_ CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion);
typedef HRESULT(STDMETHODCALLTYPE* SwapChainPresentType)(IDirect3DSwapChain9* s, THIS_ CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion, DWORD dwFlags);
typedef HRESULT(STDMETHODCALLTYPE* ResetType)(IDirect3DDevice9* d, THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters);
typedef HRESULT(STDMETHODCALLTYPE* PresentExType)(IDirect3DDevice9Ex* d, THIS_ CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion, DWORD dwFlags);
typedef HRESULT(STDMETHODCALLTYPE* ResetExType)(IDirect3DDevice9Ex* d, THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters, D3DDISPLAYMODEEX*);
typedef HRESULT(STDMETHODCALLTYPE* EndSceneType)(IDirect3DDevice9* d);

