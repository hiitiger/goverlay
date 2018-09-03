#pragma once

namespace
{
static const char k_overlayIWindow[] = "n_overlay_win_x0y1x2";
}

typedef IDirect3D9 *(WINAPI *pFnDirect3DCreate9)(UINT SDKVersion);
typedef HRESULT(WINAPI *pFnDirect3DCreate9Ex)(UINT, void **);

typedef HRESULT(WINAPI *pFnCreateDXGIFactory1)(REFIID riid, void **ppFactory);

typedef HRESULT(STDMETHODCALLTYPE *PresentType)(IDirect3DDevice9 *d, THIS_ CONST RECT *pSourceRect, CONST RECT *pDestRect, HWND hDestWindowOverride, CONST RGNDATA *pDirtyRegion);
typedef HRESULT(STDMETHODCALLTYPE *SwapChainPresentType)(IDirect3DSwapChain9 *s, THIS_ CONST RECT *pSourceRect, CONST RECT *pDestRect, HWND hDestWindowOverride, CONST RGNDATA *pDirtyRegion, DWORD dwFlags);
typedef HRESULT(STDMETHODCALLTYPE *ResetType)(IDirect3DDevice9 *d, THIS_ D3DPRESENT_PARAMETERS *pPresentationParameters);
typedef HRESULT(STDMETHODCALLTYPE *PresentExType)(IDirect3DDevice9Ex *d, THIS_ CONST RECT *pSourceRect, CONST RECT *pDestRect, HWND hDestWindowOverride, CONST RGNDATA *pDirtyRegion, DWORD dwFlags);
typedef HRESULT(STDMETHODCALLTYPE *ResetExType)(IDirect3DDevice9Ex *d, THIS_ D3DPRESENT_PARAMETERS *pPresentationParameters, D3DDISPLAYMODEEX *);
typedef HRESULT(STDMETHODCALLTYPE *EndSceneType)(IDirect3DDevice9 *d);

typedef HRESULT(WINAPI *pFnD3D11CreateDeviceAndSwapChain)(
    __in_opt IDXGIAdapter *pAdapter,
    D3D_DRIVER_TYPE DriverType,
    HMODULE Software,
    UINT Flags,
    __in_ecount_opt(FeatureLevels) CONST D3D_FEATURE_LEVEL *pFeatureLevels,
    UINT FeatureLevels,
    UINT SDKVersion,
    __in_opt CONST DXGI_SWAP_CHAIN_DESC *pSwapChainDesc,
    __out_opt IDXGISwapChain **ppSwapChain,
    __out_opt ID3D11Device **ppDevice,
    __out_opt D3D_FEATURE_LEVEL *pFeatureLevel,
    __out_opt ID3D11DeviceContext **ppImmediateContext);

typedef HRESULT(WINAPI *pFnD3D10CreateDeviceAndSwapChain)(
    IDXGIAdapter *pAdapter,
    D3D10_DRIVER_TYPE DriverType,
    HMODULE Software,
    UINT Flags,
    UINT SDKVersion,
    DXGI_SWAP_CHAIN_DESC *pSwapChainDesc,
    IDXGISwapChain **ppSwapChain,
    ID3D10Device **ppDevice);

typedef HRESULT(STDMETHODCALLTYPE *DXGISwapChainPresentType)(IDXGISwapChain *, UINT, UINT);
typedef HRESULT(STDMETHODCALLTYPE *DXGISwapChainResizeBuffersType)(IDXGISwapChain *, UINT, UINT, UINT, DXGI_FORMAT, UINT);
typedef HRESULT(STDMETHODCALLTYPE *DXGISwapChainResizeTargetType)(IDXGISwapChain *, const DXGI_MODE_DESC *);
typedef HRESULT(STDMETHODCALLTYPE *DXGISwapChainPresent1Type)(IDXGISwapChain1 *swapChain, UINT SyncInterval, UINT PresentFlags, _In_ const DXGI_PRESENT_PARAMETERS *pPresentParameters);
