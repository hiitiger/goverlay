#include "StdAfx.h"
#include "D9Graphics.h"


D9Graphics::D9Graphics(void)
    : _d3d9(nullptr)
    , _device(nullptr)
    , _width(0)
    , _height(0)
{

}

D9Graphics::~D9Graphics(void)
{
}

bool D9Graphics::initD3d9(HWND window)
{
    _window = window;
    //1. Get d3d9 interface
    _d3d9 = Direct3DCreate9(D3D_SDK_VERSION);

    //2. check device capability
    D3DCAPS9 caps;
    HRESULT hr = _d3d9->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps);

    int vp = 0;
    if (caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
    {
        vp = D3DCREATE_HARDWARE_VERTEXPROCESSING;
    }
    else
    {
        vp = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
    }
    //3. D3DPRESENT_PARAMETERS
    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory(&d3dpp, sizeof(D3DPRESENT_PARAMETERS));
    d3dpp.BackBufferWidth = _width;
    d3dpp.BackBufferHeight = _height;
    d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;
    d3dpp.BackBufferCount = 1;
    d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
    d3dpp.MultiSampleQuality = 0;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.hDeviceWindow = window;
    d3dpp.Windowed = true;
    d3dpp.EnableAutoDepthStencil = true;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8; //depth format
    d3dpp.Flags = 0;
    d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
    d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

    //4. CREATE IDerect3DDevice9
    hr = _d3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, window, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, _device.resetAndGetPointerAddress());
    if ((FAILED(hr)))
    {
        switch (hr)
        {
        case D3DERR_DEVICELOST:
            OutputDebugStringW(L"D3DERR_DEVICELOST");
            break;
        case D3DERR_INVALIDCALL:
            OutputDebugStringW(L"D3DERR_INVALIDCALL");
            break;
        case D3DERR_NOTAVAILABLE:
            OutputDebugStringW(L"D3DERR_NOTAVAILABLE");
            break;
        case D3DERR_OUTOFVIDEOMEMORY:
            OutputDebugStringW(L"D3DERR_OUTOFVIDEOMEMORY");
            break;

        }
        return false;
    }



    return true;
}

void D9Graphics::shutdown()
{
    _model.release();

    _device = nullptr;
    _d3d9 = nullptr;
}

void D9Graphics::resize(int w, int h)
{
    _width = w;
    _height = h;

    _model.release();
    _texRT = nullptr;

    if (_device)
    {
        //3. D3DPRESENT_PARAMETERS
        D3DPRESENT_PARAMETERS d3dpp;
        ZeroMemory(&d3dpp, sizeof(D3DPRESENT_PARAMETERS));
        d3dpp.BackBufferWidth = _width;
        d3dpp.BackBufferHeight = _height;
        d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;
        d3dpp.BackBufferCount = 1;
        d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
        d3dpp.MultiSampleQuality = 0;
        d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
        d3dpp.hDeviceWindow = _window;
        d3dpp.Windowed = true;
        d3dpp.EnableAutoDepthStencil = true;
        d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8; //depth format
        d3dpp.Flags = 0;
        d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
        d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
        _device->Reset(&d3dpp);

        setup();
    }
}

void D9Graphics::setup()
{
    _device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    _device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    _device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_POINT);

    _model.setupModel(_device);

}

void D9Graphics::render(float timeDelta)
{
    if (_device)
    {
        _device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xfff0f0ff, 1.0f, 0);

        D3DXVECTOR3 pos(4.0f, 4.0f, -26.0f);
        D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
        D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);

        D3DXMATRIX V;
        D3DXMatrixLookAtLH(
            &V,
            &pos,
            &target,
            &up);

        _device->SetTransform(D3DTS_VIEW, &V);

        D3DXMATRIX proj;
        D3DXMatrixPerspectiveFovLH(
            &proj,
            D3DX_PI * 0.5f, // 90 - degree
            (float)_width / (float)_height,
            1.0f,
            1000.0f);
        _device->SetTransform(D3DTS_PROJECTION, &proj);

        _device->SetRenderState(D3DRS_NORMALIZENORMALS, true);
        _device->SetRenderState(D3DRS_SPECULARENABLE, true);
        _device->SetRenderState(D3DRS_LIGHTING, true);

        D3DXVECTOR3 dir(1.0f, -1.0f, 1.0f);
        D3DXCOLOR col(1.0f, 1.0f, 1.0f, 1.0f);
        D3DLIGHT9 light = D3DUtils::initDirectionLight(&dir, &col);

        _device->SetLight(0, &light);
        _device->LightEnable(0, true);


        _device->BeginScene();

        static float y = 0.0f;
        D3DXMATRIX yRot;
        D3DXMatrixRotationY(&yRot, y);
        y += timeDelta;

        if (y >= 6.28f)
            y = 0.0f;

        D3DXMATRIX World = yRot;

        _model.render(&World, (GetAsyncKeyState(0x51) & 0x8000) != 0);

        _device->EndScene();

        _device->Present(0, 0, 0, 0);

    }
}

