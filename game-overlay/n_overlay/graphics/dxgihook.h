#pragma once
#include "graphics.h"

struct DXGIHookData
{
    HMODULE dxgiModule_ = nullptr;
    HMODULE d3d10Module_ = nullptr;
    HMODULE d3d11Module_ = nullptr;

    std::unique_ptr<ApiHook<DXGISwapChainPresentType>> dxgiSwapChainPresentHook_;
    std::unique_ptr<ApiHook<DXGISwapChainResizeBuffersType>> dxgiSwapChainResizeBuffersHook_;
    std::unique_ptr<ApiHook<DXGISwapChainResizeTargetType>> dxgiSwapChainResizeTargetHook_;
    std::unique_ptr<ApiHook<DXGISwapChainPresent1Type>> dxgiSwapChainPresent1Hook_;
};

class DxgiGraphics;

class DXGIHook : public IHookModule, public DXGIHookData
{
    bool graphicsInit_ = false;

    bool dxgiLibraryLinked_ = false;

    pFnD3D11CreateDeviceAndSwapChain d3d11Create_ = nullptr;
    pFnD3D10CreateDeviceAndSwapChain d3d10Create_ = nullptr;

    std::unique_ptr<DxgiGraphics> dxgiGraphics_;

  public:
    DXGIHook();
    ~DXGIHook();

    bool hook() override;
    void unhook() override;

    HRESULT STDMETHODCALLTYPE Present_hook(IDXGISwapChain *swap, UINT SyncInterval, UINT Flags);

    HRESULT STDMETHODCALLTYPE Present1_hook(IDXGISwapChain1 *swap, UINT SyncInterval, UINT PresentFlags, _In_ const DXGI_PRESENT_PARAMETERS *pPresentParameters);

    HRESULT STDMETHODCALLTYPE ResizeBuffers_hook(IDXGISwapChain *swap,
                                                 UINT BufferCount,
                                                 UINT Width,
                                                 UINT Height,
                                                 DXGI_FORMAT NewFormat,
                                                 UINT SwapChainFlags);

    HRESULT STDMETHODCALLTYPE ResizeTarget_hook(IDXGISwapChain *swap, __in const DXGI_MODE_DESC *pNewTargetParameters);

  private:
    bool loadLibInProc();

    bool linkDX10Library();
    bool linkDX11Library();

    bool tryHookDXGI();

    bool hookSwapChain(Windows::ComPtr<IDXGISwapChain>);

  private:
    void onBeforePresent(IDXGISwapChain *);
    void onAfterPresent(IDXGISwapChain *);
    void onResize(IDXGISwapChain *);

  private:
    bool initGraphics(IDXGISwapChain *swap);
    void uninitGraphics(IDXGISwapChain *swap);
    void freeGraphics();
};