#pragma once

class DxgiGraphics : public CommonGraphics
{
private:

    FpsTimer fpsTimer_;
public:
    virtual ~DxgiGraphics() {}

    bool isWindowed() const;

    virtual Windows::ComPtr<IDXGISwapChain> swapChain() const = 0;

    virtual bool initGraphics(IDXGISwapChain *swap);

    virtual void uninitGraphics(IDXGISwapChain *swap);
    virtual void freeGraphics();

    virtual void beforePresent(IDXGISwapChain *swap);
    virtual void afterPresent(IDXGISwapChain *swap) ;

    virtual bool _initGraphicsContext(IDXGISwapChain* swap) = 0;
    virtual bool _initGraphicsState() = 0;
    virtual void _initSpriteDrawer() = 0;

    virtual void _saveStatus() = 0;
    virtual void _prepareStatus() = 0;
    virtual void _restoreStatus() = 0;
};
typedef HRESULT(WINAPI* pFnD3D11CreateDeviceAndSwapChain)(
    __in_opt IDXGIAdapter* pAdapter,
    D3D_DRIVER_TYPE DriverType,
    HMODULE Software,
    UINT Flags,
    __in_ecount_opt(FeatureLevels) CONST D3D_FEATURE_LEVEL* pFeatureLevels,
    UINT FeatureLevels,
    UINT SDKVersion,
    __in_opt CONST DXGI_SWAP_CHAIN_DESC* pSwapChainDesc,
    __out_opt IDXGISwapChain** ppSwapChain,
    __out_opt ID3D11Device** ppDevice,
    __out_opt D3D_FEATURE_LEVEL* pFeatureLevel,
    __out_opt ID3D11DeviceContext** ppImmediateContext);

typedef HRESULT(WINAPI* pFnD3D10CreateDeviceAndSwapChain)(
    IDXGIAdapter* pAdapter,
    D3D10_DRIVER_TYPE DriverType,
    HMODULE Software,
    UINT Flags,
    UINT SDKVersion,
    DXGI_SWAP_CHAIN_DESC* pSwapChainDesc,
    IDXGISwapChain** ppSwapChain,
    ID3D10Device** ppDevice);

typedef HRESULT(STDMETHODCALLTYPE* DXGISwapChainPresentType)(IDXGISwapChain*, UINT, UINT);
typedef HRESULT(STDMETHODCALLTYPE* DXGISwapChainResizeBuffersType)(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT);
typedef HRESULT(STDMETHODCALLTYPE* DXGISwapChainResizeTargetType)(IDXGISwapChain*, const DXGI_MODE_DESC*);
typedef HRESULT(STDMETHODCALLTYPE* DXGISwapChainPresent1Type)(IDXGISwapChain1* swapChain, UINT SyncInterval, UINT PresentFlags, _In_ const DXGI_PRESENT_PARAMETERS* pPresentParameters);

inline DXGI_FORMAT fixCopyTextureFormat(DXGI_FORMAT format)
{
    switch (format)
    {
    case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB: return DXGI_FORMAT_B8G8R8A8_UNORM;
    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB: return DXGI_FORMAT_R8G8B8A8_UNORM;
    }

    return format;
}


inline bool isSRGBFormat(DXGI_FORMAT format)
{
    return format == DXGI_FORMAT_B8G8R8A8_UNORM_SRGB ||
        format == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
}