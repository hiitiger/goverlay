#pragma once
#include "graphics.h"

struct DXGIHookData
{
    HMODULE dxgiModule_ = nullptr;
    HMODULE d3d10Module_ = nullptr;
    HMODULE d3d11Module_ = nullptr;
    HMODULE d3d12Module_ = nullptr;

    std::unique_ptr<ApiHook<DXGISwapChainPresentType>> dxgiSwapChainPresentHook_;
    std::unique_ptr<ApiHook<DXGISwapChainResizeBuffersType>> dxgiSwapChainResizeBuffersHook_;
    std::unique_ptr<ApiHook<DXGISwapChainResizeTargetType>> dxgiSwapChainResizeTargetHook_;
    std::unique_ptr<ApiHook<DXGISwapChainPresent1Type>> dxgiSwapChainPresent1Hook_;
    std::unique_ptr<ApiHook<D3D12ExecuteCommandListsType>> d3d12ExecuteCommandListsHook_;
};

class DxgiGraphics;

class DXGIHook : public IHookModule, public DXGIHookData
{
    bool graphicsInit_ = false;

    bool dxgiLibraryLinked_ = false;

    pFnD3D11CreateDeviceAndSwapChain d3d11Create_ = nullptr;
    pFnD3D10CreateDeviceAndSwapChain d3d10Create_ = nullptr;
    PFN_D3D12_CREATE_DEVICE d3d12Create_ = nullptr;

    std::unique_ptr<DxgiGraphics> dxgiGraphics_;
    ID3D12CommandQueue* d3d12CommandQueue_ = nullptr;

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

    HRESULT STDMETHODCALLTYPE ExecuteCommandLists_hook(ID3D12CommandQueue* queue, UINT NumCommandLists, ID3D12CommandList* const* ppCommandLists);
  
private:
    bool loadLibInProc();

    bool linkDX10Library();
    bool linkDX11Library();
    bool linkDX12Library();

    bool tryHookD3D12();
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