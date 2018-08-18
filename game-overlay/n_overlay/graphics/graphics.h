#pragma once


namespace
{
    static const char k_overlayIWindow[] = "n_overlay_win_x0y1x2";
}

struct IGraphicsHook
{
    virtual ~IGraphicsHook() { }

    virtual bool hook() = 0;
    virtual void unhook() = 0;
};


typedef IDirect3D9* (WINAPI* pFnDirect3DCreate9)(UINT SDKVersion);
typedef HRESULT(WINAPI *pFnDirect3DCreate9Ex)(UINT, void **);

typedef HRESULT(WINAPI *pFnCreateDXGIFactory1)(REFIID riid, void **ppFactory);


typedef HRESULT(STDMETHODCALLTYPE* PresentType)(IDirect3DDevice9* d, THIS_ CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion);
typedef HRESULT(STDMETHODCALLTYPE* SwapChainPresentType)(IDirect3DSwapChain9* s, THIS_ CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion, DWORD dwFlags);
typedef HRESULT(STDMETHODCALLTYPE* ResetType)(IDirect3DDevice9* d, THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters);
typedef HRESULT(STDMETHODCALLTYPE* PresentExType)(IDirect3DDevice9Ex* d, THIS_ CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion, DWORD dwFlags);
typedef HRESULT(STDMETHODCALLTYPE* ResetExType)(IDirect3DDevice9Ex* d, THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters, D3DDISPLAYMODEEX*);
typedef HRESULT(STDMETHODCALLTYPE* EndSceneType)(IDirect3DDevice9* d);
