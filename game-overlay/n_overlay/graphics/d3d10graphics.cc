#include "stable.h"
#include "overlay/hookapp.h"
#include "overlay/overlay.h"
#include "overlay/session.h"

#include "d3d10sprite.h"
#include "D3d10Graphics.h"

D3d10Graphics::D3d10Graphics()
{

}

D3d10Graphics::~D3d10Graphics()
{
    freeGraphics();
}


Windows::ComPtr<IDXGISwapChain> D3d10Graphics::swapChain() const
{
    return swap_;
}

void D3d10Graphics::freeGraphics()
{
    __super::freeGraphics();
    ZeroMemory(&savedStatus_, sizeof(savedStatus_));

    statusBarSprite_ = nullptr;
    mainSprite_ = nullptr;
    windowSprites_.clear();

    blockSprite_ = nullptr;

    rasterizeState_ = nullptr;
    transparentBlendState_ = nullptr;
    depthStencilState_ = nullptr;

    renderTargetView_ = nullptr;
    d3dDevice_ = nullptr;
    swap_ = nullptr;
}


bool D3d10Graphics::_initGraphicsContext(IDXGISwapChain* swap)
{
    swap_ = swap;

    HRESULT hr;

    hr = swap->GetDevice(__uuidof(ID3D10Device), (LPVOID*)d3dDevice_.resetAndGetPointerAddress());

    if (!d3dDevice_)
    {
        return false;
    }

    Windows::ComPtr<ID3D10Texture2D> backBufferTexture;
    swap->GetBuffer(0, __uuidof(ID3D10Texture2D), (LPVOID*)backBufferTexture.resetAndGetPointerAddress());
    if (!backBufferTexture)
    {
        return false;
    }

    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    swap->GetDesc(&swapChainDesc);

    D3D10_TEXTURE2D_DESC textureDesc;
    backBufferTexture->GetDesc(&textureDesc);

    targetWidth_ = textureDesc.Width;
    targetHeight_ = textureDesc.Height;
    dxgiformat_ = fixCopyTextureFormat(swapChainDesc.BufferDesc.Format);

    hr = d3dDevice_->CreateRenderTargetView(backBufferTexture, nullptr, renderTargetView_.resetAndGetPointerAddress());
    if (!renderTargetView_)
    {
        return false;
    }

    return true;
}

bool D3d10Graphics::_initGraphicsState()
{
    HRESULT hr;
    D3D10_DEPTH_STENCIL_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.DepthEnable = false;
    desc.DepthWriteMask = D3D10_DEPTH_WRITE_MASK_ALL;
    desc.DepthFunc = D3D10_COMPARISON_LESS;
    desc.StencilEnable = false;
    desc.StencilReadMask = 0xff;
    desc.StencilWriteMask = 0xff;

    hr = d3dDevice_->CreateDepthStencilState(&desc, depthStencilState_.resetAndGetPointerAddress());
    if (!depthStencilState_)
    {
        return false;
    }

    D3D10_BLEND_DESC transDesc;
    ZeroMemory(&transDesc, sizeof(transDesc));
    transDesc.AlphaToCoverageEnable = FALSE;
    transDesc.BlendEnable[0] = TRUE;
    transDesc.SrcBlend = D3D10_BLEND_SRC_ALPHA;
    transDesc.DestBlend = D3D10_BLEND_INV_SRC_ALPHA;
    transDesc.BlendOp = D3D10_BLEND_OP_ADD;
    transDesc.SrcBlendAlpha = D3D10_BLEND_ZERO;
    transDesc.DestBlendAlpha = D3D10_BLEND_ZERO;
    transDesc.BlendOpAlpha = D3D10_BLEND_OP_ADD;
    transDesc.RenderTargetWriteMask[0] = D3D10_COLOR_WRITE_ENABLE_ALL;

    hr = d3dDevice_->CreateBlendState(&transDesc, transparentBlendState_.resetAndGetPointerAddress());
    if (!transparentBlendState_)
    {
        return false;
    }

    D3D10_RASTERIZER_DESC rDesc = {};
    rDesc.CullMode = D3D10_CULL_BACK;
    rDesc.FillMode = D3D10_FILL_SOLID;
    rDesc.FrontCounterClockwise = false;
    rDesc.DepthClipEnable = true;
    hr = d3dDevice_->CreateRasterizerState(&rDesc, rasterizeState_.resetAndGetPointerAddress());
    if (!rasterizeState_)
    {
        return false;
    }

    return true;
}

void D3d10Graphics::_initSpriteDrawer()
{
    sprite_.reset(new D3d10SpriteDrawer(d3dDevice_));
    sprite_->init(session::loadModuleD3dCompiler47(), targetWidth_, targetHeight_);
}

void D3d10Graphics::_createSprites()
{
    D3D10_TEXTURE2D_DESC textureDesc;
    textureDesc.Width = targetWidth_;
    textureDesc.Height = targetHeight_;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;

    textureDesc.Usage = D3D10_USAGE_DYNAMIC;
    textureDesc.BindFlags = D3D10_BIND_SHADER_RESOURCE;
    textureDesc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
    textureDesc.MiscFlags = 0;

    if (SUCCEEDED(d3dDevice_->CreateTexture2D(&textureDesc, nullptr, blockSprite_.resetAndGetPointerAddress())))
    {
        D3D10_MAPPED_TEXTURE2D ms = {};
        blockSprite_->Map(0, D3D10_MAP_WRITE_DISCARD, NULL, &ms);
        int* bytePointer = (int*)ms.pData;
        memset(bytePointer, 0xff, ms.RowPitch * textureDesc.Height);
        blockSprite_->Unmap(0);
    }
}

void D3d10Graphics::_createWindowSprites()
{
    HookApp::instance()->overlayConnector()->lockWindows();

    auto windows = HookApp::instance()->overlayConnector()->windows();

    for (const auto& w : windows)
    {
        auto windowSprite = _createWindowSprite(w);
        if (windowSprite)
        {
            if (w->name == "MainOverlay")
                mainSprite_ = windowSprite;
            else if (w->name == "StatusBar")
                statusBarSprite_ = windowSprite;
            else if(w->name == "OverlayTip")
                overlayTipSprite_ = windowSprite;
            windowSprites_.push_back(windowSprite);
        }
    }

    HookApp::instance()->overlayConnector()->unlockWindows();
}

Windows::ComPtr<ID3D10Texture2D> D3d10Graphics::_createDynamicTexture(std::uint32_t width, std::uint32_t height)
{
    Windows::ComPtr<ID3D10Texture2D> texture;
    D3D10_TEXTURE2D_DESC textureDesc;
    textureDesc.Width = width;
    textureDesc.Height = height;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;

    textureDesc.Usage = D3D10_USAGE_DYNAMIC;
    textureDesc.BindFlags = D3D10_BIND_SHADER_RESOURCE;
    textureDesc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
    textureDesc.MiscFlags = 0;

    HRESULT hr = d3dDevice_->CreateTexture2D(&textureDesc, nullptr, texture.resetAndGetPointerAddress());
    if (FAILED(hr))
    {
        LOGGER("n_overlay") << L"CreateTexture2D, failed:" << hr;
        std::cout << "D3d10Graphics::_createDynamicTexture CreateTexture2D, failed:" << hr << std::endl;
        return nullptr;
    }

    return texture;
}

std::shared_ptr<D3d10WindowSprite> D3d10Graphics::_createWindowSprite(const std::shared_ptr<overlay::Window>& window)
{
    std::shared_ptr<D3d10WindowSprite> windowSprite = std::make_shared<D3d10WindowSprite>();
    windowSprite->windowId = window->windowId;
    windowSprite->name = window->name;
    windowSprite->bufferName = window->bufferName;
    windowSprite->rect = window->rect;

    windowSprite->texture = _createDynamicTexture(window->rect.width, window->rect.height);
    if (!windowSprite->texture)
    {
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

void D3d10Graphics::_updateSprite(std::shared_ptr<D3d10WindowSprite>& windowSprite, bool clear /*= false*/)
{
    if (!windowSprite->texture)
    {
        return;
    }

    D3D10_MAPPED_TEXTURE2D ms = {};
    HRESULT hr = windowSprite->texture->Map(0, D3D10_MAP_WRITE_DISCARD, NULL, &ms);
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
    HookApp::instance()->overlayConnector()->unlockShareMem();

    windowSprite->texture->Unmap(0);
}

void D3d10Graphics::_checkAndResyncWindows()
{
    if (needResync_)
    {
        std::lock_guard<std::mutex> lock(synclock_);
        if (pendingWindows_.size() > 0 || pendingFrameBufferUpdates_.size() > 0)
        {
            HookApp::instance()->overlayConnector()->lockWindows();

            auto windows = HookApp::instance()->overlayConnector()->windows();

            for (auto windowId : pendingWindows_)
            {
                auto it = std::find_if(windows.begin(), windows.end(), [windowId](const auto &window) {
                    return windowId == window->windowId;
                });
                if (it != windows.end())
                {
                    if (auto windowSprite = _createWindowSprite(*it))
                    {

                        if ((*it)->name == "MainOverlay")
                            mainSprite_ = windowSprite;
                        else if ((*it)->name == "StatusBar")
                            statusBarSprite_ = windowSprite;
                        else if ((*it)->name == "OverlayTip")
                            overlayTipSprite_ = windowSprite;
                        windowSprites_.push_back(windowSprite);
                    }
                }
            }

            pendingWindows_.clear();

            for (auto windowId : pendingFrameBufferUpdates_)
            {
                auto it = std::find_if(windowSprites_.begin(), windowSprites_.end(), [windowId](const auto &window) {
                    return windowId == window->windowId;
                });
                if (it != windowSprites_.end())
                {
                    auto& windowSprite = *it;
                    try
                    {
                        windowSprite->windowBitmapMem.reset(new boost::interprocess::windows_shared_memory(boost::interprocess::open_only, windowSprite->bufferName.c_str(), boost::interprocess::read_only));
                        windowSprite->fullRegion.reset(new boost::interprocess::mapped_region(*windowSprite->windowBitmapMem, boost::interprocess::read_only));
                    }
                    catch (...)
                    {
                    }
                }
            }

            HookApp::instance()->overlayConnector()->unlockWindows();
        }

        if (pendingClosed_.size() > 0)
        {
            for (auto windowId : pendingClosed_)
            {
                auto it = std::find_if(windowSprites_.begin(), windowSprites_.end(), [windowId](const auto &window) {
                    return windowId == window->windowId;
                });
                if (it != windowSprites_.end())
                {
                    if ((*it)->name == "MainOverlay")
                        mainSprite_ = nullptr;
                    else if ((*it)->name == "StatusBar")
                        statusBarSprite_ = nullptr;
                    else if ((*it)->name == "OverlayTip")
                        overlayTipSprite_ = nullptr;
                    windowSprites_.erase(it);
                }
            }

            pendingClosed_.clear();
        }

        if (pendingBounds_.size() > 0)
        {
            for (const auto&[windowId, rect] : pendingBounds_)
            {
                auto it = std::find_if(windowSprites_.begin(), windowSprites_.end(), [windowId](const auto &window) {
                    return windowId == window->windowId;
                });
                if (it != windowSprites_.end())
                {
                    auto& windowSprite = *it;
                    windowSprite->rect = rect;

                    D3D10_TEXTURE2D_DESC desc = { 0 };

                    if (windowSprite->texture)
                    {
                        windowSprite->texture->GetDesc(&desc);
                    }

                    if (desc.Width == windowSprite->rect.width
                        && desc.Height == windowSprite->rect.height)
                    {
                        continue;
                    }
                    else if (desc.Width < (UINT)windowSprite->rect.width
                        || desc.Height < (UINT)windowSprite->rect.height)
                    {
                        //create a new larger texture

                        windowSprite->texture = _createDynamicTexture(windowSprite->rect.width, windowSprite->rect.height);
                        if (!windowSprite->texture)
                        {
                            windowSprites_.erase(it);
                            continue;
                        }

                        _updateSprite(windowSprite, true);
                    }
                    else
                    {
                        _updateSprite(windowSprite, true);
                    }
                }
            }
            pendingBounds_.clear();
        }

        if (pendingFrameBuffers_.size() > 0)
        {
            for (auto windowId : pendingFrameBuffers_)
            {
                auto it = std::find_if(windowSprites_.begin(), windowSprites_.end(), [windowId](const auto &window) {
                    return windowId == window->windowId;
                });

                if (it != windowSprites_.end())
                {
                    _updateSprite(*it);
                }
            }

            pendingFrameBuffers_.clear();
        }

        if (focusWindowId_)
        {
            if (windowSprites_.at(windowSprites_.size() - 1)->windowId != focusWindowId_)
            {
                auto it = std::find_if(windowSprites_.begin(), windowSprites_.end(), [&](const auto& w) {
                    return w->windowId == focusWindowId_;
                });
                if (it != windowSprites_.end())
                {
                    auto focusWindow = *it;
                    windowSprites_.erase(it);
                    windowSprites_.push_back(focusWindow);
                }
            }
        }

        needResync_ = false;
    }
}

void D3d10Graphics::_drawBlockSprite()
{
    RECT  drawRect = { 0, 0, (LONG)targetWidth_, (LONG)targetHeight_ };
    sprite_->drawUnscaleSprite(blockSprite_, drawRect, 0x800c0c0c);
}

void D3d10Graphics::_drawWindowSprites()
{
    for (auto& windowSprite : windowSprites_)
    {
#if 0
        if (windowSprite->name == "MainOverlay")
            continue;
#endif
        if (windowSprite->name == "StatusBar")
            continue;
        if (windowSprite->name == "PopupTip")
            continue;

        _drawWindowSprite(windowSprite);
    }
}

void D3d10Graphics::_drawMainSprite()
{
    if (mainSprite_)
    {
        _drawWindowSprite(mainSprite_);
    }
}

void D3d10Graphics::_drawStatutBarSprite()
{
    if (statusBarSprite_)
    {
        _drawWindowSprite(statusBarSprite_);
    }
}

void D3d10Graphics::_drawPopupTipSprite()
{
    if (overlayTipSprite_)
    {
        overlayTipSprite_->rect.x = targetWidth_ - overlayTipSprite_->rect.width - 10;
        overlayTipSprite_->rect.y = targetHeight_ - overlayTipSprite_->rect.height - 10;
        _drawWindowSprite(overlayTipSprite_);
    }
}

void D3d10Graphics::_drawWindowSprite(std::shared_ptr<D3d10WindowSprite>& windowSprite)
{
    RECT  drawRect = { windowSprite->rect.x, windowSprite->rect.y, windowSprite->rect.x + windowSprite->rect.width , windowSprite->rect.y + windowSprite->rect.height };

    sprite_->drawUnscaleSprite(windowSprite->texture, drawRect);
}

void D3d10Graphics::_saveStatus()
{
    d3dDevice_->IAGetInputLayout(&savedStatus_.input_layout);
    d3dDevice_->IAGetVertexBuffers(0, 1, &savedStatus_.vertex, &savedStatus_ .vertex_stride, &savedStatus_.vertex_offset);
    d3dDevice_->IAGetPrimitiveTopology((D3D10_PRIMITIVE_TOPOLOGY *)&savedStatus_.draw_style);

    d3dDevice_->VSGetShader(&savedStatus_.vertex_shader);
    d3dDevice_->PSGetShader(&savedStatus_.pixel_shader);
    d3dDevice_->VSGetConstantBuffers(0, 1, &savedStatus_.const_buffer);
    d3dDevice_->PSGetShaderResources(0, 1, &savedStatus_.shader_view);
    d3dDevice_->OMGetDepthStencilState(&savedStatus_.depth_stencil_state,
        &savedStatus_.stencil_ref);
    d3dDevice_->OMGetBlendState(&savedStatus_.blend_state, savedStatus_.blen_factor,
        &savedStatus_.blen_mask);
    d3dDevice_->OMGetRenderTargets(1, &savedStatus_.render_target,
        &savedStatus_.depth_stencil);

    UINT number = D3D10_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
    d3dDevice_->RSGetViewports(&number, savedStatus_.view_port);
    savedStatus_.view_port_nums = number;

    d3dDevice_->RSGetState(&savedStatus_.rasterizer);
    d3dDevice_->PSGetSamplers(0, 1, &savedStatus_.sampler_states);
}

void D3d10Graphics::_prepareStatus()
{
    d3dDevice_->OMSetDepthStencilState(depthStencilState_, 0);

    float factor[4] = { 0.f, 0.f, 0.f, 0.f };
    d3dDevice_->OMSetBlendState(transparentBlendState_, factor, 0xffffffff);

    d3dDevice_->OMSetRenderTargets(1, renderTargetView_.getPointerAdress(), nullptr);

    d3dDevice_->RSSetState(rasterizeState_);


    D3D10_VIEWPORT viewport;
    ZeroMemory(&viewport, sizeof(D3D10_VIEWPORT));

    viewport.Width = targetWidth_;
    viewport.Height = targetHeight_;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;

    d3dDevice_->RSSetViewports(1, &viewport);
}

void D3d10Graphics::_restoreStatus()
{
    d3dDevice_->IASetInputLayout(savedStatus_.input_layout);
    d3dDevice_->IASetVertexBuffers(0, 1, &savedStatus_.vertex, &savedStatus_.vertex_stride, &savedStatus_.vertex_offset);
    d3dDevice_->IASetPrimitiveTopology((D3D10_PRIMITIVE_TOPOLOGY)savedStatus_.draw_style);

    d3dDevice_->VSSetShader(savedStatus_.vertex_shader);
    d3dDevice_->PSSetShader(savedStatus_.pixel_shader);
    d3dDevice_->PSSetSamplers(0, 1, &savedStatus_.sampler_states);

    d3dDevice_->VSSetConstantBuffers(0, 1, &savedStatus_.const_buffer);
    d3dDevice_->PSSetShaderResources(0, 1, &savedStatus_.shader_view);
    d3dDevice_->OMSetDepthStencilState(savedStatus_.depth_stencil_state,
        savedStatus_.stencil_ref);
    d3dDevice_->OMSetBlendState(savedStatus_.blend_state,
        savedStatus_.blen_factor, savedStatus_.blen_mask);
    d3dDevice_->OMSetRenderTargets(1, &savedStatus_.render_target,
        savedStatus_.depth_stencil);

    d3dDevice_->RSSetViewports(savedStatus_.view_port_nums, savedStatus_.view_port);

    d3dDevice_->RSSetState(savedStatus_.rasterizer);

    ReleaseCOM(savedStatus_.rasterizer);
    ReleaseCOM(savedStatus_.input_layout);
    ReleaseCOM(savedStatus_.vertex);
    ReleaseCOM(savedStatus_.vertex_shader);
    ReleaseCOM(savedStatus_.pixel_shader);
    ReleaseCOM(savedStatus_.const_buffer);
    ReleaseCOM(savedStatus_.shader_view);
    ReleaseCOM(savedStatus_.depth_stencil_state);
    ReleaseCOM(savedStatus_.blend_state);
    ReleaseCOM(savedStatus_.render_target);
    ReleaseCOM(savedStatus_.depth_stencil);
    ReleaseCOM(savedStatus_.sampler_states);

    ZeroMemory(&savedStatus_, sizeof(savedStatus_));
}

