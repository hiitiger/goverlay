#pragma once

#include "dxgigraphics.h"
#include <dxgi1_4.h>

#define MAX_BACKBUFFERS 8

struct BuackBufferInfo {
    ID3D12Resource* backbuffer[MAX_BACKBUFFERS];
    UINT count;
};

struct D3d12WindowSprite
{
    int windowId;
    std::string name;
    overlay::WindowRect rect;
    std::string bufferName;

    std::unique_ptr<windows_shared_memory> windowBitmapMem;

    Windows::ComPtr<ID3D11Texture2D> texture;
};

// Resources that are referenced by descriptor handles (a.k.a. resource views).
struct HandledResource {
    union {
        D3D12_CPU_DESCRIPTOR_HANDLE descHandle;
        D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle;
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;
    };

    ID3D12Resource* resource;

    ~HandledResource() { resource->Release(); }
};


class D3d11SpriteDrawer;

class D3d12Graphics : public DxgiGraphics
{
    std::unique_ptr<D3d11SpriteDrawer> sprite_;

    Windows::ComPtr<IDXGISwapChain> swap_;
    Windows::ComPtr<IDXGISwapChain3> swap3_;

    Windows::ComPtr<ID3D11On12Device> d3d11On12Device_;
    Windows::ComPtr<ID3D12Device> d3d12Device_;
    Windows::ComPtr<ID3D11Device> d3d11Device_;
    Windows::ComPtr<ID3D11DeviceContext> d3d11Context_;
    Windows::ComPtr<ID3D12CommandQueue> d3d12CommandQueue_;
    std::array<Windows::ComPtr<ID3D11Resource>, MAX_BACKBUFFERS> backBufferTextureList_;
    std::array<Windows::ComPtr<ID3D11RenderTargetView>, MAX_BACKBUFFERS>  renderTargetViewList_;

    BuackBufferInfo m_bbInfo;
    bool isDxgi_1_4_;

    DXGI_FORMAT dxgiformat_ = DXGI_FORMAT_UNKNOWN;
    bool isSRGB_ = false;

    Windows::ComPtr<ID3D11DepthStencilState> depthStencilState_;
    Windows::ComPtr<ID3D11BlendState> transparentBlendState_;
    Windows::ComPtr<ID3D11RasterizerState> rasterizeState_;
    Windows::ComPtr<ID3D11Texture2D> blockSprite_;

    struct D3d11Status
    {
        UINT view_port_nums;
        D3D11_VIEWPORT view_port[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
        ID3D11InputLayout* input_layout;
        ID3D11VertexShader* vertex_shader;
        ID3D11PixelShader* pixel_shader;
        ID3D11ShaderResourceView* shader_view;

        ID3D11SamplerState* sampler_states;

        ID3D11Buffer* const_buffer;
        ID3D11Buffer* vertex;
        UINT vertex_stride;
        UINT vertex_offset;
        ID3D11DepthStencilState* depth_stencil_state;
        UINT stencil_ref;
        DWORD draw_style;
        ID3D11BlendState* blend_state;
        float blen_factor[4];
        UINT blen_mask;
        ID3D11RenderTargetView* render_target;
        ID3D11DepthStencilView* depth_stencil;
        ID3D11RasterizerState* rasterizer;
    };

    D3d11Status savedStatus_ = { 0 };

    UINT curr_back_buffer_ = 0;

    std::vector<std::shared_ptr<D3d12WindowSprite>> windowSprites_;
    std::shared_ptr<D3d12WindowSprite> mainSprite_;
    std::shared_ptr<D3d12WindowSprite> statusBarSprite_;
    std::shared_ptr<D3d12WindowSprite> overlayTipSprite_;

public:
    D3d12Graphics();
    ~D3d12Graphics();

    Windows::ComPtr<IDXGISwapChain> swapChain() const override;
    void freeGraphics() override;
    void setCommandQueue(ID3D12CommandQueue* queue);

    bool _initGraphicsContext(IDXGISwapChain* swap) override;
    bool _initGraphicsState() override;
    void _initSpriteDrawer() override;

    void _createSprites() override;
    void _createWindowSprites() override;

    Windows::ComPtr<ID3D11Texture2D> _createDynamicTexture(std::uint32_t width, std::uint32_t height);
    std::shared_ptr<D3d12WindowSprite> _createWindowSprite(const std::shared_ptr<overlay::Window>& window);
    void _updateSprite(std::shared_ptr<D3d12WindowSprite>& sprite, bool clear = false);

    void _checkAndResyncWindows() override;

    void _drawBlockSprite() override;
    void _drawWindowSprites() override;
    void _drawMainSprite() override;
    void _drawStatutBarSprite() override;
    void _drawPopupTipSprite() override;

    void _drawWindowSprite(std::shared_ptr<D3d12WindowSprite>&);

    void _saveStatus() override;
    void _prepareStatus() override;
    void _restoreStatus() override;
};
