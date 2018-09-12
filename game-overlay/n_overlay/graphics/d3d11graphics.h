#pragma once

#include "dxgigraphics.h"

struct D3d11WindowSprite
{
    int windowId;
    std::string name;
    overlay::WindowRect rect;
    std::string bufferName;

    std::unique_ptr<boost::interprocess::windows_shared_memory> windowBitmapMem;
    std::unique_ptr<boost::interprocess::mapped_region> fullRegion;

    Windows::ComPtr<ID3D11Texture2D> texture;
};


class D3d11SpriteDrawer;

class D3d11Graphics : public DxgiGraphics
{

    std::unique_ptr<D3d11SpriteDrawer> sprite_;

    Windows::ComPtr<IDXGISwapChain> swap_;

    Windows::ComPtr<ID3D11Device> m_d3dDevice;
    Windows::ComPtr<ID3D11DeviceContext> m_d3dContext;
    Windows::ComPtr<ID3D11RenderTargetView> m_renderTargetView;

    std::uint32_t m_targetWidth;
    std::uint32_t m_targetHeight;
    DXGI_FORMAT m_dxgiformat;

    Windows::ComPtr<ID3D11DepthStencilState> m_depthStencilState;
    Windows::ComPtr<ID3D11BlendState> m_transparentBS;
    Windows::ComPtr<ID3D11RasterizerState> m_rasterizeState;

    Windows::ComPtr<ID3D11Texture2D> m_clickHintTexture;


    struct D3d11Status
    {
        UINT view_port_nums;
        D3D11_VIEWPORT view_port[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
        ID3D11InputLayout * input_layout;
        ID3D11VertexShader * vertex_shader;
        ID3D11PixelShader * pixel_shader;
        ID3D11ShaderResourceView * shader_view;

        ID3D11SamplerState* sampler_states;

        ID3D11Buffer * const_buffer;
        ID3D11Buffer * vertex;
        UINT vertex_stride;
        UINT vertex_offset;
        ID3D11DepthStencilState * depth_stencil_state;
        UINT stencil_ref;
        DWORD draw_style;
        ID3D11BlendState * blend_state;
        float blen_factor[4];
        UINT blen_mask;
        ID3D11RenderTargetView * render_target;
        ID3D11DepthStencilView * depth_stencil;
        ID3D11RasterizerState *  rasterizer;
    };

    D3d11Status m_savesStatus;


    std::shared_ptr<D3d11WindowSprite> mainSprite_;

public:
    D3d11Graphics();
    ~D3d11Graphics();

    bool initGraphics(IDXGISwapChain* swap) override;

    bool _initGraphicsContext(IDXGISwapChain* swap);
    bool _initGraphicsState();

    void uninitGraphics(IDXGISwapChain* swap) override;
    void freeGraphics() override;

    void beforePresent(IDXGISwapChain* swap) override;
    void afterPresent(IDXGISwapChain* swap) override;

    void _createSprites();
    void _createWindowSprites();

    std::shared_ptr<D3d11WindowSprite> _createWindowSprite(const std::shared_ptr<overlay::Window>& window);
    void _updateSprite(std::shared_ptr<D3d11WindowSprite>& sprite, bool clear = false);


    void _drawBlockSprite();
    void _drawMainSprite();

    void _drawWindowSprite(std::shared_ptr<D3d11WindowSprite>&);

    void _saveStatus();
    void _prepareStatus();
    void _restoreStatus();
};