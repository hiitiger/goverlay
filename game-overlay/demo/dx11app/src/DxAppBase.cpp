/*
    Beginning DirectX 11 Game Programming
    By Allen Sherrod and Wendy Jones

    DirectX 11 Base Class - Used as base class for all DirectX 11 demos in this book.
    */

#include "stable.h"
#include "GameTimer.h"
#include "DxAppBase.h"

DxAppBase* G_App = nullptr;

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    return G_App->WindowProc(hwnd, message, wParam, lParam);
}

DxAppBase::DxAppBase()
    : mInstance(nullptr)
    , mWindow(nullptr)
    , m_fps(0)
    , mAppPaused(false)
    , mClientWidth(0)
    , mClientHeight(0)
    , mDriverType(D3D_DRIVER_TYPE_HARDWARE)
    , mFeatureLevel(D3D_FEATURE_LEVEL_11_0)
    , m4xMsaaEnable(false)
    , m4xMsaaQuality(0)
    , md3dDevice(nullptr)
    , md3dContext(nullptr)
    , mSwapChain(nullptr)
    , mRenderTargetView(nullptr)
    , mDepthStencilBuffer(nullptr)
    , mDepthStencilView(nullptr)
{
    G_App = this;
}


DxAppBase::~DxAppBase()
{
    Shutdown();
}


bool DxAppBase::Init(HINSTANCE instance)
{
    mInstance = instance;
    if (!InitWindow())
    {
        return false;
    }

    if (!InitDirect3D(mWindow))
    {
        return false;
    }
    return true;
}

bool DxAppBase::InitWindow()
{
    WNDCLASSEX wndClass = { 0 };
    wndClass.cbSize = sizeof(WNDCLASSEX);
    wndClass.style = CS_HREDRAW | CS_VREDRAW;
    wndClass.lpfnWndProc = WndProc;
    wndClass.hInstance = mInstance;
    wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wndClass.lpszMenuName = NULL;
    wndClass.lpszClassName = L"DX11BookWindowClass";

    if (!RegisterClassEx(&wndClass))
    {
        return false;
    }

    RECT rc = { 0, 0, 600, 600 };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

    mWindow = CreateWindowW(L"DX11BookWindowClass", L"Blank Direct3D 11 Window", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top,
        NULL, NULL, mInstance, NULL);

    return mWindow != nullptr;
}

bool DxAppBase::InitDirect3D(HWND window)
{
    mWindow = window;

    RECT dimensions;
    GetClientRect(window, &dimensions);

    mClientWidth = dimensions.right - dimensions.left;
    mClientHeight = dimensions.bottom - dimensions.top;


    Windows::ComPtr<IDXGIFactory1> dxgiFactory;
    HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(dxgiFactory.resetAndGetPointerAddress()));
    if (FAILED(hr)) {
        return false;
    }

    Windows::ComPtr<IDXGIAdapter> adapter;
    hr = dxgiFactory->EnumAdapters(0, adapter.resetAndGetPointerAddress());
    if (FAILED(hr)){
        return false;
    }

    DXGI_ADAPTER_DESC desc;
    adapter->GetDesc(&desc);
    m_gpudescription = desc.Description;

    //1.Create device and device context
    unsigned int creationFlags = 0;
    //#ifdef _DEBUG
    //    creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
    //#endif

    hr = D3D11CreateDevice(
        adapter,          // default adapter
        D3D_DRIVER_TYPE_UNKNOWN,
        nullptr,          // no software device
        creationFlags,
        nullptr, 0,       // default feature level array
        D3D11_SDK_VERSION,
        &md3dDevice,
        &mFeatureLevel,
        &md3dContext);
    if (FAILED(hr))
    {
        DXTRACE_MSG(L"Failed to create the Direct3D device!");
        return false;
    }
    if (mFeatureLevel != D3D_FEATURE_LEVEL_11_0)
    {
        DXTRACE_MSG(L"Direct3D Feature Level 11 unsupported.");
        return false;
    }

    //2.Check 4x MSAA Quality support
    HRERROR(md3dDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &m4xMsaaQuality));
    assert(m4xMsaaQuality > 0);

    //3.Describe our Buffer
    DXGI_MODE_DESC bufferDesc;
    ZeroMemory(&bufferDesc, sizeof(DXGI_MODE_DESC));

    // one back buffer
    bufferDesc.Width = mClientWidth;
    bufferDesc.Height = mClientHeight;
    bufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    bufferDesc.RefreshRate.Numerator = 60;
    bufferDesc.RefreshRate.Denominator = 1;
    bufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    bufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    //4.Describe our SwapChain
    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

    swapChainDesc.BufferDesc = bufferDesc;
    if (m4xMsaaEnable)
    {
        swapChainDesc.SampleDesc.Count = 4;
        swapChainDesc.SampleDesc.Quality = m4xMsaaQuality - 1;
    }
    else
    {
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
    }
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = 1;
    swapChainDesc.OutputWindow = window;
    swapChainDesc.Windowed = TRUE;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;    // allow full-screen switching

    //4.Create our SwapChain

    HRERROR(dxgiFactory->CreateSwapChain(md3dDevice, &swapChainDesc, &mSwapChain));
    // Release our acquired COM interfaces (because we are done with them).

    initRenderState();

    mCamera.SetPosition(0.0f, 0.0f, -10.0f);

    OnResize();

    m_spriteDrawer.reset(new DX11Sprite(md3dDevice, md3dContext));
    m_spriteDrawer->init();

    HRERROR(FW1CreateFactory(FW1_VERSION, m_fw1FontFactory.resetAndGetPointerAddress()));
    HRERROR(m_fw1FontFactory->CreateFontWrapper(md3dDevice, L"Arial", m_fwFontWrapper.resetAndGetPointerAddress()));

    return loadFPS() && LoadContent();
}


void DxAppBase::ShowGameWindow()
{
    ::ShowWindow(mWindow, SW_SHOWNORMAL);
}

void DxAppBase::Shutdown()
{
    unloadFPS();
    UnloadContent();

    pTransparentBS = nullptr;
    m_spriteDrawer.reset(nullptr);

    ReleaseCOM(mDepthStencilBuffer);
    ReleaseCOM(mDepthStencilView);

    ReleaseCOM(mRenderTargetView);
    ReleaseCOM(mSwapChain);

    if (md3dContext)
    {
        md3dContext->ClearState();
    }

    ReleaseCOM(md3dContext);
    ReleaseCOM(md3dDevice);
}

void DxAppBase::initRenderState()
{
    D3D11_BLEND_DESC transDesc;

    transDesc.AlphaToCoverageEnable = false;        //关闭AlphaToCoverage  
    transDesc.IndependentBlendEnable = false;       //不针对多个RenderTarget使用不同的混合状态  
    transDesc.RenderTarget[0].BlendEnable = true;
    transDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    transDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    transDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    transDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    transDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    transDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    transDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    md3dDevice->CreateBlendState(&transDesc, pTransparentBS.resetAndGetPointerAddress());


    D3D11_BLEND_DESC opaqueDesc;
    opaqueDesc.AlphaToCoverageEnable = false;
    opaqueDesc.AlphaToCoverageEnable = false;        //关闭AlphaToCoverage  
    opaqueDesc.IndependentBlendEnable = false;       //不针对多个RenderTarget使用不同的混合状态  
    opaqueDesc.RenderTarget[0].BlendEnable = false;
    opaqueDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    opaqueDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    opaqueDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    opaqueDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    opaqueDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    opaqueDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    opaqueDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    md3dDevice->CreateBlendState(&opaqueDesc, pOpaqueBS.resetAndGetPointerAddress());
}


bool DxAppBase::LoadContent()
{
    // Override with demo specifics, if any...
    return true;
}

void DxAppBase::UnloadContent()
{
    // Override with demo specifics, if any...
}

void DxAppBase::UpdateScene(float)
{

}

void DxAppBase::DrawScene()
{
    md3dContext->ClearRenderTargetView(mRenderTargetView, D3DXCOLOR(Convert::ToXmColor(Colors::LightSteelBlue)));
    md3dContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void DxAppBase::PresentScene()
{
    mSwapChain->Present(0, 0);
}

void DxAppBase::CalcFPS()
{
    static int frameCnt = 0;
    static float timeElapsed = 0.0f;

    frameCnt++;

    // Compute averages over one second period.
    if ((mTimer.GameTime() - timeElapsed) >= 1.0f)
    {
        m_fps = frameCnt; // fps = frameCnt / 1
       // float mspf = 1000.0f / fps;

        std::wstring fpsStr = std::to_wstring((long long)m_fps) + L", GPU:" + m_gpudescription;

        SetWindowText(mWindow, fpsStr.c_str());

        // Reset for next average.
        frameCnt = 0;
        timeElapsed += 1.0f;
    }
}

SIZE DxAppBase::renderTargetSize()
{
    SIZE sz = { 0 };
    if (md3dContext)
    {

        Windows::ComPtr<ID3D11RenderTargetView> renderTargetView;
        md3dContext->OMGetRenderTargets(
            1,
            renderTargetView.resetAndGetPointerAddress(),
            nullptr
            );

        Windows::ComPtr<ID3D11Resource> renderTarget;
        renderTargetView->GetResource(renderTarget.resetAndGetPointerAddress());

        Windows::ComPtr<ID3D11Texture2D> renderTargetTexture;
        renderTarget->QueryInterface(renderTargetTexture.resetAndGetPointerAddress());

        D3D11_TEXTURE2D_DESC renderTargetTextureDesc;
        renderTargetTexture->GetDesc(&renderTargetTextureDesc);

        sz.cx = renderTargetTextureDesc.Width;
        sz.cy = renderTargetTextureDesc.Height;
    }
    return sz;
}

void DxAppBase::enableAlpha( )
{
    md3dContext->OMGetBlendState(savedBS.BS.resetAndGetPointerAddress(), savedBS.blen_factor, &savedBS.blen_mask);

    float factor[4] = { 1.f, 1.f, 1.f, 1.f };
    md3dContext->OMSetBlendState(pTransparentBS, factor, 0xffffffff);
 
}

void DxAppBase::restoreAlpha()
{
    md3dContext->OMSetBlendState(savedBS.BS, savedBS.blen_factor, savedBS.blen_mask);
}

int DxAppBase::RunLoop()
{
    ShowGameWindow();

    mTimer.Reset();
    MSG msg = { 0 };

    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            mTimer.Tick();

            if (!mAppPaused)
            {
                CalcFPS();

                UpdateScene(mTimer.DeltaTime());
                DrawScene();
                drawFPS();

                PresentScene();
            }
            else
            {
                //  Sleep(0);
            }
        }
    }

    Shutdown();

    return static_cast<int>(msg.wParam);
}


LRESULT DxAppBase::WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_ACTIVATE:
        if (LOWORD(wParam) == WA_INACTIVE)
        {
            mAppPaused = true;
            mTimer.Stop();
        }
        else
        {
            mAppPaused = false;
            mTimer.Start();
        }
        break;
    case WM_SIZE:
        // Save the new client area dimensions.
        mClientWidth = LOWORD(lParam);
        mClientHeight = HIWORD(lParam);
        if (mClientHeight != 0 || mClientWidth != 0)
        {
            OnResize();
        }
        break;
    case WM_GETMINMAXINFO:
        ((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
        ((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
        OnMouseDown(wParam, LOWORD(lParam), HIWORD(lParam));
        return 0;
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
        OnMouseUp(wParam, LOWORD(lParam), HIWORD(lParam));
        return 0;
    case WM_MOUSEMOVE:
        OnMouseMove(wParam, LOWORD(lParam), HIWORD(lParam));
        return 0;

    case WM_KEYDOWN:
    {
        if (wParam == VK_ESCAPE)
        {
            WCHAR szPath[MAX_PATH] = { 0 };
            GetModuleFileNameW(NULL, szPath, MAX_PATH);
            WCHAR achLongPath[MAX_PATH] = { 0 };
            TCHAR **lppPart = { NULL };
            ::GetFullPathNameW(szPath, MAX_PATH, achLongPath, lppPart);

            std::wstring dirPath = achLongPath;
            size_t lastSepartor = dirPath.find_last_of('\\');
            dirPath.erase(lastSepartor);

#ifdef _WIN64
            dirPath.append(L"\\n_overlay.x64.dll");
#else
            dirPath.append(L"\\n_overlay.dll");
#endif
            ::LoadLibraryW(dirPath.c_str());
        }
    }

    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }

    return 0;
}

void DxAppBase::OnMouseDown(WPARAM btnState, int x, int y)
{
    mLastMousePos.x = x;
    mLastMousePos.y = y;

    SetCapture(mWindow);
}

void DxAppBase::OnMouseUp(WPARAM btnState, int x, int y)
{
    ReleaseCapture();
}

void DxAppBase::OnMouseMove(WPARAM btnState, int x, int y)
{
    if ((btnState & MK_LBUTTON) != 0)
    {
        // Make each pixel correspond to a quarter of a degree.
        float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
        float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePos.y));

        mCamera.Pitch(dy);
        mCamera.RotateY(dx);
    }

    mLastMousePos.x = x;
    mLastMousePos.y = y;
}

void DxAppBase::OnResize()
{
    if (!mSwapChain)
    {
        return;
    }

    ReleaseCOM(mRenderTargetView);
    ReleaseCOM(mDepthStencilView);
    ReleaseCOM(mDepthStencilBuffer);

    HR(mSwapChain->ResizeBuffers(1, mClientWidth, mClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0));

    //Create our BackBuffer
    Windows::ComPtr<ID3D11Texture2D> backBufferTexture;
    HR(mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)backBufferTexture.resetAndGetPointerAddress()));

    //5.Create our Render Target View
    HR(md3dDevice->CreateRenderTargetView(backBufferTexture, 0, &mRenderTargetView));

    //6.Depth Stencil buffer
    D3D11_TEXTURE2D_DESC depthStencilDesc;
    depthStencilDesc.Width = mClientWidth;
    depthStencilDesc.Height = mClientHeight;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.ArraySize = 1;
    depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    if (m4xMsaaEnable)
    {
        depthStencilDesc.SampleDesc.Count = 4;
        depthStencilDesc.SampleDesc.Quality = m4xMsaaQuality - 1;
    }
    else
    {
        depthStencilDesc.SampleDesc.Count = 1;
        depthStencilDesc.SampleDesc.Quality = 0;
    }
    depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
    depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthStencilDesc.CPUAccessFlags = 0;
    depthStencilDesc.MiscFlags = 0;

    HR(md3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, &mDepthStencilBuffer));
    HR(md3dDevice->CreateDepthStencilView(mDepthStencilBuffer, nullptr, &mDepthStencilView));

    //7.Set our Render Target
    md3dContext->OMSetRenderTargets(1, &mRenderTargetView, mDepthStencilView);
    // md3dContext->OMSetRenderTargets(1, &mRenderTargetView, nullptr);

     //8.Set Viewport
    D3D11_VIEWPORT viewport;
    ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

    viewport.Width = static_cast<float>(mClientWidth);
    viewport.Height = static_cast<float>(mClientHeight);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;

    md3dContext->RSSetViewports(1, &viewport);

    float  fieldOfView = (float)XM_PI / 4.0f;
    float screenAspect = (float)mClientWidth / (float)mClientHeight;
    mCamera.SetLens(fieldOfView, screenAspect, 1.0f, 1000.0f);

}

bool DxAppBase::loadFPS()
{
    D3DX11_IMAGE_LOAD_INFO loadInfo;
    ZeroMemory(&loadInfo, sizeof(D3DX11_IMAGE_LOAD_INFO));
    loadInfo.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    loadInfo.Format = DXGI_FORMAT_BC2_UNORM;
    loadInfo.MipLevels = D3DX11_DEFAULT;
    loadInfo.MipFilter = D3DX11_FILTER_LINEAR;

    HRERROR(D3DX11CreateShaderResourceViewFromFile(md3dDevice, L"FPS.png", &loadInfo, NULL, m_fpstextureView.resetAndGetPointerAddress(), NULL));

    Windows::ComPtr<ID3D11Resource> res;
    m_fpstextureView->GetResource(res.resetAndGetPointerAddress());

    HRERROR(res->QueryInterface<ID3D11Texture2D>(m_fpsTexture.resetAndGetPointerAddress()));

    m_spriteDrawer->addTexture(m_fpsTexture, m_fpstextureView);

    return true;
}

void DxAppBase::unloadFPS()
{
    m_fpsTexture = nullptr;
    m_fpstextureView = nullptr;
}

void DxAppBase::drawFPS()
{
    enableAlpha();
    md3dContext->RSSetState(nullptr);

    static int fpsShow = 0;
    if (fpsShow < m_fps)
    {
        fpsShow += 1;
    }
    else if (fpsShow > m_fps)
    {
        fpsShow -= 1;
    }

    Windows::ComPtr<IFW1TextGeometry> fw1TextGeometry;
    m_fw1FontFactory->CreateTextGeometry(fw1TextGeometry.resetAndGetPointerAddress());
    
    FW1_RECTF rect;

    rect.Left = 10.0; rect.Right = 100.0;
    rect.Top = 10.0;  rect.Bottom = 40.0;

    D3D11StateSaver save;

    save.saveCurrentState(md3dContext);

    m_fwFontWrapper->AnalyzeString(
        md3dContext,
        std::to_wstring((long long)fpsShow).c_str(),
        L"Arial",
        24.0f,
        &rect,
        0xff0099ff,
        0,
        fw1TextGeometry);


    m_fwFontWrapper->DrawGeometry(
        md3dContext,
        fw1TextGeometry,
        nullptr,
        nullptr,
        0
        );

    save.restoreSavedState();
    
    if (m_spriteDrawer)
    {
        m_spriteDrawer->begin();

        int fps = fpsShow;
        int pos0 = fps % 10;
        int pos1 = (fps / 10) % 10;
        int pos2 = (fps / 100) % 10;
        int pos3 = (fps / 1000) % 10;
        int pos4 = (fps / 10000) % 10;

        if (pos4 > 0)
        {
            pos3 = pos2 = pos1 = pos0 = 9;
        }

        int index = 0;

        int drawWidth = 24;
        int drawHeight = 32;

        if (pos3 > 0)
        {
            RECT src = calcFpsTexRect(m_fpsTexture, pos3);
            RECT dst = { 0, 0, drawWidth, drawHeight };
            m_spriteDrawer->drawScaleSprite(m_fpsTexture, src, dst);
            index += 1;
        }

        if (index > 0 || pos2 > 0)
        {
            RECT src = calcFpsTexRect(m_fpsTexture, pos2);
            RECT dst = { index * drawWidth, 0, index * drawWidth + drawWidth, drawHeight };
            m_spriteDrawer->drawScaleSprite(m_fpsTexture, src, dst);
            index += 1;
        }

        if (index > 0 || pos1 > 0)
        {
            RECT src = calcFpsTexRect(m_fpsTexture, pos1);
            RECT dst = { index * drawWidth, 0, index * drawWidth + drawWidth, drawHeight };
            m_spriteDrawer->drawScaleSprite(m_fpsTexture, src, dst);
            index += 1;
        }


        RECT src = calcFpsTexRect(m_fpsTexture, pos0);
        RECT dst = { index * drawWidth, 0, index * drawWidth + drawWidth, drawHeight };
        m_spriteDrawer->drawScaleSprite(m_fpsTexture, src, dst);

        m_spriteDrawer->end();
    }

    restoreAlpha();
}

