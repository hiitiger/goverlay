#include "stable.h"
#include "overlay/hookapp.h"
#include "overlay/overlay.h"
#include "overlay/session.h"
#include "d3d11sprite.h"
#include "D3d11Graphics.h"

D3d11Graphics::D3d11Graphics()
{

}

D3d11Graphics::~D3d11Graphics()
{
    freeGraphics();
}

bool D3d11Graphics::initGraphics(IDXGISwapChain* swap)
{
    DAssert(!swap_);
    bool succcedd = _initGraphicsContext(swap) && _initGraphicsState();

    if (succcedd)
    {
        sprite_.reset(new D3d11SpriteDrawer(m_d3dDevice, m_d3dContext));
        sprite_->init(session::loadModuleD3dCompiler47(), m_targetWidth, m_targetHeight);

        _createSprites();
        _createWindowSprites();
    }

    if (!succcedd)
    {
        freeGraphics();
    }

    return succcedd;
}

bool D3d11Graphics::_initGraphicsContext(IDXGISwapChain* swap)
{
    swap_ = swap;

    HRESULT hr;

    hr = swap->GetDevice(__uuidof(ID3D11Device), (LPVOID*)m_d3dDevice.resetAndGetPointerAddress());

    if (!m_d3dDevice)
    {
        return false;
    }

    m_d3dDevice->GetImmediateContext(m_d3dContext.resetAndGetPointerAddress());

    Windows::ComPtr<ID3D11Texture2D> backBufferTexture;
    swap->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)backBufferTexture.resetAndGetPointerAddress());
    if (!backBufferTexture)
    {
        return false;
    }

    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    swap->GetDesc(&swapChainDesc);

    D3D11_TEXTURE2D_DESC textureDesc;
    backBufferTexture->GetDesc(&textureDesc);

    m_targetWidth = textureDesc.Width;
    m_targetHeight = textureDesc.Height;
    m_dxgiformat = fixCopyTextureFormat(swapChainDesc.BufferDesc.Format);

    hr = m_d3dDevice->CreateRenderTargetView(backBufferTexture, nullptr, m_renderTargetView.resetAndGetPointerAddress());
    if (!m_renderTargetView)
    {
        return false;
    }

    return true;
}

bool D3d11Graphics::_initGraphicsState()
{
    HRESULT hr;
    D3D11_DEPTH_STENCIL_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.DepthEnable = false;
    desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    desc.DepthFunc = D3D11_COMPARISON_LESS;
    desc.StencilEnable = false;
    desc.StencilReadMask = 0xff;
    desc.StencilWriteMask = 0xff;

    hr = m_d3dDevice->CreateDepthStencilState(&desc, m_depthStencilState.resetAndGetPointerAddress());
    if (!m_depthStencilState)
    {
        return false;
    }

    D3D11_BLEND_DESC transDesc;
    transDesc.AlphaToCoverageEnable = false;
    transDesc.IndependentBlendEnable = false;
    transDesc.RenderTarget[0].BlendEnable = true;
    transDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    transDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    transDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    transDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    transDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    transDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    transDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    hr = m_d3dDevice->CreateBlendState(&transDesc, m_transparentBS.resetAndGetPointerAddress());
    if (!m_transparentBS)
    {
        return false;
    }

    D3D11_RASTERIZER_DESC rDesc = {};
    rDesc.CullMode = D3D11_CULL_BACK;
    rDesc.FillMode = D3D11_FILL_SOLID;
    rDesc.FrontCounterClockwise = false;
    rDesc.DepthClipEnable = true;
    m_d3dDevice->CreateRasterizerState(&rDesc, m_rasterizeState.resetAndGetPointerAddress());

    if (!m_rasterizeState)
    {
        return false;
    }

    return true;
}

void D3d11Graphics::uninitGraphics(IDXGISwapChain* swap)
{
    if (swap_.get() != swap)
    {
        return;
    }

    freeGraphics();
}

void D3d11Graphics::freeGraphics()
{
    ZeroMemory(&m_savesStatus, sizeof(m_savesStatus));

    mainSprite_ = nullptr;

    m_clickHintTexture = nullptr;

    m_rasterizeState = nullptr;
    m_transparentBS = nullptr;
    m_depthStencilState = nullptr;

    m_renderTargetView = nullptr;
    m_d3dContext = nullptr;
    m_d3dDevice = nullptr;
    swap_ = nullptr;
}

void D3d11Graphics::beforePresent(IDXGISwapChain* swap)
{
    if (swap_.get() != swap)
    {
        return;
    }

    _saveStatus();
    _prepareStatus();

    //if (HookApp::instance()->uiapp()->isInterceptingInput())
    {
        _drawBlockSprite();
    }

    _drawMainSprite();

    _restoreStatus();
}

void D3d11Graphics::afterPresent(IDXGISwapChain* swap)
{
    if (swap_.get() != swap)
    {
        return;
    }


}

void D3d11Graphics::_createSprites()
{
    D3D11_TEXTURE2D_DESC textureDesc;
    textureDesc.Width = m_targetWidth;
    textureDesc.Height = m_targetHeight;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;

    textureDesc.Usage = D3D11_USAGE_DYNAMIC;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    textureDesc.MiscFlags = 0;

    if (SUCCEEDED(m_d3dDevice->CreateTexture2D(&textureDesc, nullptr, m_clickHintTexture.resetAndGetPointerAddress())))
    {
        D3D11_MAPPED_SUBRESOURCE ms;
        m_d3dContext->Map(m_clickHintTexture, 0, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
        int* bytePointer = (int*)ms.pData;
        memset(bytePointer, 0xff, ms.RowPitch * textureDesc.Height);
        m_d3dContext->Unmap(m_clickHintTexture, 0);
    }


}

void D3d11Graphics::_createWindowSprites()
{
    HookApp::instance()->overlayConnector()->lockWindows();

    auto windows = HookApp::instance()->overlayConnector()->windows();

    for (const auto& w : windows)
    {
        if (w->name == "MainOverlay")
        {
            mainSprite_ = _createWindowSprite(w);
        }
    }

    HookApp::instance()->overlayConnector()->unlockWindows();

}

std::shared_ptr<D3d11WindowSprite> D3d11Graphics::_createWindowSprite(const std::shared_ptr<overlay::Window>& window)
{
    std::shared_ptr<D3d11WindowSprite> windowSprite = std::make_shared<D3d11WindowSprite>();
    windowSprite->windowId = window->windowId;
    windowSprite->name = window->name;
    windowSprite->bufferName = window->bufferName;
    windowSprite->rect = window->rect;

    D3D11_TEXTURE2D_DESC textureDesc;
    textureDesc.Width = window->rect.width;
    textureDesc.Height = window->rect.height;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;

    textureDesc.Usage = D3D11_USAGE_DYNAMIC;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    textureDesc.MiscFlags = 0;

    HRESULT hr = m_d3dDevice->CreateTexture2D(&textureDesc, nullptr, windowSprite->texture.resetAndGetPointerAddress());
    if (FAILED(hr))
    {
        LOGGER("n_overlay") << L"CreateTexture2D, failed:" << hr;
        std::cout << "Dx11Engine::createSprite CreateTexture2D, failed:" << hr << std::endl;
        return nullptr;
    }

    try
    {
        windowSprite->windowBitmapMem.reset(new boost::interprocess::windows_shared_memory(boost::interprocess::open_only, windowSprite->bufferName.c_str(), boost::interprocess::read_only));
        windowSprite->fullRegion.reset(new boost::interprocess::mapped_region(*windowSprite->windowBitmapMem, boost::interprocess::read_only));
    }
    catch (...)
    {
        return nullptr;
    }

    _updateSprite(windowSprite, true);
    return windowSprite;
}

void D3d11Graphics::_updateSprite(std::shared_ptr<D3d11WindowSprite>& windowSprite, bool clear /*= false*/)
{
    if (!windowSprite->texture)
    {
        return;
    }

    D3D11_MAPPED_SUBRESOURCE ms = {};
    HRESULT hr = m_d3dContext->Map(windowSprite->texture, 0, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
    if (FAILED(hr))
    {
        LOGGER("n_overlay") << L"update sprite failed:" << hr;
    }

    if (!ms.pData)
    {
        return;
    }

    int* bytePointer = (int*)ms.pData;

    if (clear)
    {
        memset(bytePointer, 0, sizeof(int) * windowSprite->rect.width * windowSprite->rect.height);
    }

    HookApp::instance()->overlayConnector()->lockShareMem();
    char *orgin = static_cast<char*>(windowSprite->fullRegion->get_address());
    if (orgin)
    {
        overlay::ShareMemFrameBuffer* head = (overlay::ShareMemFrameBuffer*)orgin;
        int width = std::min(windowSprite->rect.width, head->width);
        int height = std::min(windowSprite->rect.height, head->height);

        std::uint32_t* mem = (std::uint32_t*)(orgin + sizeof(overlay::ShareMemFrameBuffer));
        int spriteWidth = ms.RowPitch / sizeof(std::uint32_t);

        for (int i = 0; i != height; ++i)
        {
            const std::uint32_t* line = mem + i * head->width;
            int xx = i * spriteWidth;
            memcpy((bytePointer + xx), line, sizeof(int) * width);
        }
    }

    m_d3dContext->Unmap(windowSprite->texture, 0);
}

void D3d11Graphics::_drawBlockSprite()
{
    RECT  drawRect = { 0, 0, m_targetWidth, m_targetHeight };
    sprite_->drawUnscaleSprite(m_clickHintTexture, drawRect, 0x800c0c0c);
}


void D3d11Graphics::_drawMainSprite()
{
    if (mainSprite_)
    {
        _drawWindowSprite(mainSprite_);
    }
}

void D3d11Graphics::_drawWindowSprite(std::shared_ptr<D3d11WindowSprite>& windowSprite)
{
    RECT  drawRect = { windowSprite->rect.x, windowSprite->rect.y, windowSprite->rect.x + windowSprite->rect.width , windowSprite->rect.y + windowSprite->rect.height };

    sprite_->drawUnscaleSprite(windowSprite->texture, drawRect);
}

void D3d11Graphics::_saveStatus()
{
    m_d3dContext->IAGetInputLayout(&m_savesStatus.input_layout);
    m_d3dContext->IAGetVertexBuffers(0, 1, &m_savesStatus.vertex, &m_savesStatus.vertex_stride, &m_savesStatus.vertex_offset);
    m_d3dContext->IAGetPrimitiveTopology((D3D11_PRIMITIVE_TOPOLOGY *)&m_savesStatus.draw_style);

    m_d3dContext->VSGetShader(&m_savesStatus.vertex_shader, 0, 0);
    m_d3dContext->PSGetShader(&m_savesStatus.pixel_shader, 0, 0);
    m_d3dContext->VSGetConstantBuffers(0, 1, &m_savesStatus.const_buffer);
    m_d3dContext->PSGetShaderResources(0, 1, &m_savesStatus.shader_view);
    m_d3dContext->OMGetDepthStencilState(&m_savesStatus.depth_stencil_state,
        &m_savesStatus.stencil_ref);
    m_d3dContext->OMGetBlendState(&m_savesStatus.blend_state, m_savesStatus.blen_factor,
        &m_savesStatus.blen_mask);
    m_d3dContext->OMGetRenderTargets(1, &m_savesStatus.render_target,
        &m_savesStatus.depth_stencil);

    UINT number = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
    m_d3dContext->RSGetViewports(&number, m_savesStatus.view_port);
    m_savesStatus.view_port_nums = number;

    m_d3dContext->RSGetState(&m_savesStatus.rasterizer);

    m_d3dContext->PSGetSamplers(0, 1, &m_savesStatus.sampler_states);
}

void D3d11Graphics::_prepareStatus()
{
    m_d3dContext->OMSetDepthStencilState(m_depthStencilState, 0);

    float factor[4] = { 0.f, 0.f, 0.f, 0.f };
    m_d3dContext->OMSetBlendState(m_transparentBS, factor, 0xffffffff);

    m_d3dContext->OMSetRenderTargets(1, m_renderTargetView.getPointerAdress(), nullptr);

    m_d3dContext->RSSetState(m_rasterizeState);

    D3D11_VIEWPORT viewport;
    ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

    viewport.Width = static_cast<float>(m_targetWidth);
    viewport.Height = static_cast<float>(m_targetHeight);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;

    m_d3dContext->RSSetViewports(1, &viewport);
}

void D3d11Graphics::_restoreStatus()
{

    m_d3dContext->IASetInputLayout(m_savesStatus.input_layout);
    m_d3dContext->IASetVertexBuffers(0, 1, &m_savesStatus.vertex, &m_savesStatus.vertex_stride, &m_savesStatus.vertex_offset);
    m_d3dContext->IASetPrimitiveTopology((D3D11_PRIMITIVE_TOPOLOGY)m_savesStatus.draw_style);
    m_d3dContext->VSSetShader(m_savesStatus.vertex_shader, 0, 0);
    m_d3dContext->PSSetShader(m_savesStatus.pixel_shader, 0, 0);

    m_d3dContext->PSSetSamplers(0, 1, &m_savesStatus.sampler_states);
    m_d3dContext->VSSetConstantBuffers(0, 1, &m_savesStatus.const_buffer);
    m_d3dContext->PSSetShaderResources(0, 1, &m_savesStatus.shader_view);
    m_d3dContext->OMSetDepthStencilState(m_savesStatus.depth_stencil_state,
        m_savesStatus.stencil_ref);
    m_d3dContext->OMSetBlendState(m_savesStatus.blend_state,
        m_savesStatus.blen_factor, m_savesStatus.blen_mask);
    m_d3dContext->OMSetRenderTargets(1, &m_savesStatus.render_target,
        m_savesStatus.depth_stencil);

    m_d3dContext->RSSetViewports(m_savesStatus.view_port_nums, m_savesStatus.view_port);

    m_d3dContext->RSSetState(m_savesStatus.rasterizer);

    ReleaseCOM(m_savesStatus.rasterizer);
    ReleaseCOM(m_savesStatus.input_layout);
    ReleaseCOM(m_savesStatus.vertex);
    ReleaseCOM(m_savesStatus.vertex_shader);
    ReleaseCOM(m_savesStatus.pixel_shader);
    ReleaseCOM(m_savesStatus.const_buffer);
    ReleaseCOM(m_savesStatus.shader_view);
    ReleaseCOM(m_savesStatus.depth_stencil_state);
    ReleaseCOM(m_savesStatus.blend_state);
    ReleaseCOM(m_savesStatus.render_target);
    ReleaseCOM(m_savesStatus.depth_stencil);
    ReleaseCOM(m_savesStatus.sampler_states);

    ZeroMemory(&m_savesStatus, sizeof(m_savesStatus));
}
