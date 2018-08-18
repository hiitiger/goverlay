#pragma once
#include "graphics.h"
#include "hook/apihook.hpp"


struct  D3d9HookData
{
    HMODULE d3d9Dll = nullptr;
    std::unique_ptr<ApiHook<EndSceneType> > endSceneHook;

    std::unique_ptr<ApiHook<PresentType> > presentHook;
    std::unique_ptr<ApiHook<PresentExType> > presentExHook;

    std::unique_ptr<ApiHook<SwapChainPresentType> > swapChainPresentHook;

    std::unique_ptr<ApiHook<ResetType> > resetHook;
    std::unique_ptr<ApiHook<ResetExType> > resetExHook;
};


class D3d9Hook : public IHookModule, public D3d9HookData
{
    bool hookSetup_ = false;
    int presentRecurse_ = 0;

  public:
    D3d9Hook();
    ~D3d9Hook();

    bool hook() override;
    void unhook() override;

    STDMETHOD(D3D9EndScene_hook)(IDirect3DDevice9 *d);

    STDMETHOD(Present_hook)(IDirect3DDevice9*, THIS_ CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion);
    STDMETHOD(PresentEx_hook)(IDirect3DDevice9Ex*, THIS_ CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion, DWORD dwFlags);
    STDMETHOD(SwapChainPresent_hook)(IDirect3DSwapChain9*, THIS_ CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion, DWORD dwFlags);

    STDMETHOD(Reset_hook)(IDirect3DDevice9*, THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters);
    STDMETHOD(ResetEx_hook)(IDirect3DDevice9Ex* d, THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters, D3DDISPLAYMODEEX *pFullscreenDisplayMode);

private:
    void onEndScene(IDirect3DDevice9* device);
    bool setupHooks(IDirect3DDevice9 *device);

    void onBeforePresent(IDirect3DDevice9* device, HWND hDestWindowOverride, bool isD9Ex);
    void onAfterPresent(IDirect3DDevice9* device, HWND hDestWindowOverride, bool isD9Ex);
    void onReset(IDirect3DDevice9* device);

};