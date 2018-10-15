#include "stable.h"
#include "overlay/session.h"
#include "overlay/hookapp.h"
#include "d3d9hook.h"
#include "d3d9graphics.h"


HRESULT STDMETHODCALLTYPE H_Present_hook(IDirect3DDevice9* d, THIS_ CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion)
{
    return session::d3d9Hook()->Present_hook(d, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
}

HRESULT STDMETHODCALLTYPE H_SwapChainPresent_hook(IDirect3DSwapChain9* s, THIS_ CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion, DWORD dwFlags)
{
    return session::d3d9Hook()->SwapChainPresent_hook(s, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion, dwFlags);
}

HRESULT STDMETHODCALLTYPE H_Reset_hook(IDirect3DDevice9* d, THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters)
{
    return session::d3d9Hook()->Reset_hook(d, pPresentationParameters);
}

HRESULT STDMETHODCALLTYPE H_PresentEx_hook(IDirect3DDevice9Ex* d, THIS_ CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion, DWORD dwFlags)
{
    return session::d3d9Hook()->PresentEx_hook(d, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion, dwFlags);
}

HRESULT STDMETHODCALLTYPE H_ResetEx_hook(IDirect3DDevice9Ex* d, THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters, D3DDISPLAYMODEEX *pFullscreenDisplayMode)
{
    return session::d3d9Hook()->ResetEx_hook(d, pPresentationParameters, pFullscreenDisplayMode);
}

HRESULT STDMETHODCALLTYPE H_D3D9EndScene_hook(IDirect3DDevice9 *d)
{
    return session::d3d9Hook()->D3D9EndScene_hook(d);

}

D3d9Hook::D3d9Hook()
{

}

D3d9Hook::~D3d9Hook()
{
    unhook();
}

bool D3d9Hook::hook()
{
    bool result = false;

    Windows::ComPtr<IDirect3D9> spD3D9;
    Windows::ComPtr<IDirect3DDevice9> spD3DDevice9;
    Windows::ComPtr<IDirect3DSwapChain9> spD3DSwapChain9;
    Windows::ComPtr<IDirect3D9Ex> spD3D9Ex;
    Windows::ComPtr<IDirect3DDevice9Ex> spD3DDevice9Ex;

    HWND window = nullptr;

    d3d9Dll = GetModuleHandleW(L"d3d9.dll");
    if (!d3d9Dll)
    {
        goto END;
    }

    session::d3d9HookInfo().d3d9Dll = this->d3d9Dll;

    pFnDirect3DCreate9 fnCreateD3d9 = (pFnDirect3DCreate9)GetProcAddress(d3d9Dll, "Direct3DCreate9");
    if (!fnCreateD3d9)
    {
        goto END;
    }
    spD3D9 = fnCreateD3d9(D3D_SDK_VERSION);
    if (!spD3D9)
    {
        goto END;
    }

    pFnDirect3DCreate9Ex fnEx = (pFnDirect3DCreate9Ex)GetProcAddress(d3d9Dll, "Direct3DCreate9Ex");
    if (fnEx)
    {
        LOGGER("n_overlay") << "Direct3DCreate9Ex find";

        HRESULT hr = fnEx(D3D_SDK_VERSION, (void**)spD3D9Ex.resetAndGetPointerAddress());
        if (FAILED(hr))
        {
            LOGGER("n_overlay") << "Direct3DCreate9Ex failed hr:" << hr;
        }
    }

    window = ::CreateWindowA("STATIC", k_overlayIWindow, WS_POPUP, 0, 0, 1, 1, HWND_MESSAGE, NULL, NULL, NULL);
    if (!window)
    {
        LOGGER("n_overlay") << "CreateWindowA failed err:" << GetLastError();
    }
    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory(&d3dpp, sizeof(d3dpp));
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
    d3dpp.BackBufferWidth = 2;
    d3dpp.BackBufferHeight = 2;
    d3dpp.BackBufferCount = 1;
    d3dpp.hDeviceWindow = window;
    d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

    HRESULT hr;

    if (spD3D9Ex)
    {
        hr = spD3D9Ex->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_NULLREF, window, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, NULL, spD3DDevice9Ex.resetAndGetPointerAddress());

        if (FAILED(hr))
        {
            LOGGER("n_overlay") << "CreateDeviceEx failed " << hr;
        }
        else
        {
            LOGGER("n_overlay") << "CreateDeviceEx ok";


            DWORD_PTR* endSceneAddr = getVFunctionAddr((DWORD_PTR*)spD3DDevice9Ex.get(), 168 / 4);
            DWORD_PTR* hookEndSceneAddr = (DWORD_PTR*)H_D3D9EndScene_hook;

            endSceneHook.reset(new ApiHook<EndSceneType>(L"D9DeviceEndScene", endSceneAddr, hookEndSceneAddr));
            endSceneHook->activeHook();
        }
    }
    else
    {
        hr = spD3D9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_NULLREF, window, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, spD3DDevice9.resetAndGetPointerAddress());
        if (FAILED(hr))
        {
            LOGGER("n_overlay") << "CreateDevice failed :" << hr;
            goto END;
        }
        else
        {
            LOGGER("n_overlay") << "CreateDevice ok";

            DWORD_PTR* endSceneAddr = getVFunctionAddr((DWORD_PTR*)spD3DDevice9.get(), 168 / 4);
            DWORD_PTR* hookEndSceneAddr = (DWORD_PTR*)H_D3D9EndScene_hook;

            endSceneHook.reset(new ApiHook<EndSceneType>(L"D9DeviceEndScene", endSceneAddr, hookEndSceneAddr));
            endSceneHook->activeHook();
        }
    }

    result = endSceneHook->succeed();

END:
    if (!result)
    {
        LOGGER("n_overlay") << L"d3d9 api hook failed!";
    }
    if (window)
    {
        DestroyWindow(window);
    }

    session::d3d9HookInfo().d3d9Dll = d3d9Dll;
    session::d3d9HookInfo().endSceneHooked = result;

    HookApp::instance()->overlayConnector()->sendGraphicsHookInfo(session::d3d9HookInfo());

    return result;
}

void D3d9Hook::unhook()
{
    if (presentHook)
    {
        presentHook->removeHook();
        presentHook.reset(nullptr);
    }
    if (swapChainPresentHook)
    {
        swapChainPresentHook->removeHook();
        swapChainPresentHook.reset(nullptr);
    }
    if (resetHook)
    {
        resetHook->removeHook();
        resetHook.reset(nullptr);
    }
    if (presentExHook)
    {
        presentExHook->removeHook();
        presentExHook.reset(nullptr);
    }
    if (resetExHook)
    {
        resetExHook->removeHook();
        resetExHook.reset(nullptr);
    }

    session::d3d9HookInfo().endSceneHooked = false;
    session::d3d9HookInfo().presentHooked = false;
    session::d3d9HookInfo().presentExHooked = false;
    session::d3d9HookInfo().swapChainPresentHooked = false;
    session::d3d9HookInfo().resetHooked = false;
    session::d3d9HookInfo().resetExHooked = false;

    hookSetup_ = false;
}

STDMETHODIMP D3d9Hook::D3D9EndScene_hook(IDirect3DDevice9 * pD3DDevice9)
{
    this->onEndScene(pD3DDevice9);

    return this->endSceneHook->callOrginal<HRESULT>(pD3DDevice9);

}

STDMETHODIMP D3d9Hook::Present_hook(IDirect3DDevice9* pD3DDevice9, THIS_ CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion)
{
    Windows::ComPtr<IDirect3DDevice9Ex> pD3DDevice9Ex;
    pD3DDevice9->QueryInterface(IID_PPV_ARGS(pD3DDevice9Ex.resetAndGetPointerAddress()));
    this->onBeforePresent(pD3DDevice9, hDestWindowOverride, !!pD3DDevice9Ex);

    HRESULT hr = this->presentHook->callOrginal<HRESULT>(pD3DDevice9, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);

    this->onAfterPresent(pD3DDevice9, hDestWindowOverride, !!pD3DDevice9Ex);

    return hr;
}

STDMETHODIMP D3d9Hook::PresentEx_hook(IDirect3DDevice9Ex* pD3DDevice9Ex, THIS_ CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion, DWORD dwFlags)
{
    this->onBeforePresent(pD3DDevice9Ex, hDestWindowOverride, true);

    HRESULT hr = this->presentExHook->callOrginal<HRESULT>(pD3DDevice9Ex, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion, dwFlags);

    this->onAfterPresent(pD3DDevice9Ex, hDestWindowOverride, true);

    return hr;
}

STDMETHODIMP D3d9Hook::SwapChainPresent_hook(IDirect3DSwapChain9* pD3DSwapChain9, THIS_ CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion, DWORD dwFlags)
{
    Windows::ComPtr<IDirect3DDevice9> spD3DDevice9;
    pD3DSwapChain9->GetDevice(spD3DDevice9.resetAndGetPointerAddress());

    Windows::ComPtr<IDirect3DDevice9Ex> pD3DDevice9Ex;
    spD3DDevice9->QueryInterface(IID_PPV_ARGS(pD3DDevice9Ex.resetAndGetPointerAddress()));

    this->onBeforePresent(spD3DDevice9, hDestWindowOverride, !!pD3DDevice9Ex);

    HRESULT hr = this->swapChainPresentHook->callOrginal<HRESULT>(pD3DSwapChain9, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion, dwFlags);

    this->onAfterPresent(spD3DDevice9, hDestWindowOverride, !!pD3DDevice9Ex);

    return hr;
}

STDMETHODIMP D3d9Hook::Reset_hook(IDirect3DDevice9* pD3DDevice9, THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters)
{
    this->onReset(pD3DDevice9);

    return this->resetHook->callOrginal<HRESULT>(pD3DDevice9, pPresentationParameters);
}

STDMETHODIMP D3d9Hook::ResetEx_hook(IDirect3DDevice9Ex* pD3DDevice9Ex, THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters, D3DDISPLAYMODEEX *pFullscreenDisplayMode)
{
    this->onReset(pD3DDevice9Ex);

    return this->resetExHook->callOrginal<HRESULT>(pD3DDevice9Ex, pPresentationParameters, pFullscreenDisplayMode);
}

void D3d9Hook::onEndScene(IDirect3DDevice9 * device)
{
    if (!hookSetup_)
    {
        setupHooks(device);
        hookSetup_ = true;
    }
}

bool D3d9Hook::setupHooks(IDirect3DDevice9 *device)
{
    bool result = false;

    Windows::ComPtr<IDirect3DSwapChain9> spD3DSwapChain9;

    Windows::ComPtr<IDirect3D9Ex> spD3D9Ex;
    Windows::ComPtr<IDirect3DDevice9Ex> spD3DDevice9Ex;


    HRESULT hr = device->GetSwapChain(0, spD3DSwapChain9.resetAndGetPointerAddress());

    if (FAILED(hr))
    {
        LOGGER("n_overlay") << "CreateAdditionalSwapChain failed" << hr;
        goto END;
    }

    DWORD_PTR* presentAddr = getVFunctionAddr((DWORD_PTR*)device, 17);
    DWORD_PTR* hookPresentAddr = (DWORD_PTR*)H_Present_hook;

    DWORD_PTR* swapChainPresentAddr = getVFunctionAddr((DWORD_PTR*)spD3DSwapChain9.get(), 3);
    DWORD_PTR* hookSwapChainPresentAddr = (DWORD_PTR*)H_SwapChainPresent_hook;

    DWORD_PTR* resetAddr = getVFunctionAddr((DWORD_PTR*)device, 16);
    DWORD_PTR* hookResetAddr = (DWORD_PTR*)H_Reset_hook;

    presentHook.reset(new ApiHook<PresentType>(L"PresentType", presentAddr, hookPresentAddr));
    presentHook->activeHook();

    swapChainPresentHook.reset(new ApiHook<SwapChainPresentType>(L"SwapChainPresentType", swapChainPresentAddr, hookSwapChainPresentAddr));
    swapChainPresentHook->activeHook();

    resetHook.reset(new ApiHook<ResetType>(L"ResetType", resetAddr, hookResetAddr));
    resetHook->activeHook();

    LOGGER("n_overlay") << "Hook D9Present:" << presentHook->succeed();
    LOGGER("n_overlay") << "Hook Reset:" << resetHook->succeed();
    LOGGER("n_overlay") << "Hook D9SwapChainPresent:" << swapChainPresentHook->succeed();

    if (spD3DDevice9Ex)
    {
        DWORD_PTR* presentExAddr = getVFunctionAddr((DWORD_PTR*)spD3DDevice9Ex.get(), 121);
        DWORD_PTR* hookpresentExAddr = (DWORD_PTR*)H_PresentEx_hook; 
        presentExHook.reset(new ApiHook<PresentExType>(L"PresentExType", presentExAddr, hookpresentExAddr));
        presentExHook->activeHook();
        LOGGER("n_overlay") << "Hook D9PresentEx:" << presentExHook->succeed();

        DWORD_PTR* resetExAddr = getVFunctionAddr((DWORD_PTR*)spD3DDevice9Ex.get(), 132);
        DWORD_PTR* hookresetExAddr = (DWORD_PTR*)H_ResetEx_hook;
        resetExHook.reset(new ApiHook<ResetExType>(L"ResetExType", resetExAddr, hookresetExAddr));
        resetExHook->activeHook();
        LOGGER("n_overlay") << "Hook D9ResetEx:" << resetExHook->succeed();
    }

    result = (presentHook->succeed() || swapChainPresentHook->succeed()) && resetHook->succeed();
    if (spD3DDevice9Ex)
    {
        result &= presentExHook->succeed();
        result &= resetExHook->succeed();
    }
END:
    LOGGER("n_overlay") << "D9 setupHooks result: " << result;


    session::d3d9HookInfo().endSceneHooked = result;
    session::d3d9HookInfo().presentHooked = presentHook->succeed();
    
    session::d3d9HookInfo().swapChainPresentHooked = swapChainPresentHook->succeed();
    session::d3d9HookInfo().resetHooked = resetHook->succeed();
    if (spD3DDevice9Ex)
    {
        session::d3d9HookInfo().presentExHooked = presentExHook->succeed();
        session::d3d9HookInfo().resetExHooked = resetExHook->succeed();
    }

    HookApp::instance()->overlayConnector()->sendGraphicsHookInfo(session::d3d9HookInfo());
    return result;
}

void D3d9Hook::onBeforePresent(IDirect3DDevice9* device, HWND hDestWindowOverride, bool isD9Ex)
{
    if (!presentRecurse_)
    {
        if (graphicsInit_)
        {
            if (!session::overlayConnected() || !session::overlayEnabled())
            {
                freeGraphics();
            }
        }

        if (!graphicsInit_ && session::overlayEnabled())
        {
            initGraphics(device, hDestWindowOverride, isD9Ex);
        }

        if(graphicsInit_)
        {
            graphics_->beforePresent(device);
        }
    }

    presentRecurse_ += 1;
}

void D3d9Hook::onAfterPresent(IDirect3DDevice9* device, HWND /*hDestWindowOverride*/, bool /*isD9Ex*/)
{
    presentRecurse_ -= 1;

    if (!presentRecurse_)
    {
        if (graphics_)
        {
            graphics_->afterPresent(device);
        }
    }
}

void D3d9Hook::onReset(IDirect3DDevice9* device)
{
    uninitGraphics(device);
}

bool D3d9Hook::initGraphics(IDirect3DDevice9* device, HWND hDestWindowOverride, bool isD9Ex)
{
    Windows::ComPtr<IDirect3DSwapChain9> spSwapChain = 0;
    HRESULT hr = device->GetSwapChain(0, spSwapChain.resetAndGetPointerAddress());
    if (FAILED(hr))
    {
        return false;
    }

    D3DPRESENT_PARAMETERS desc = {0};
    hr = spSwapChain->GetPresentParameters(&desc);
    if (FAILED(hr))
    {
        return false;
    }

    HWND graphicsWindow = desc.hDeviceWindow;
    if (graphicsWindow != session::graphicsWindow())
    {
        if (!session::injectWindow())
        {
            session::setGraphicsWindow(graphicsWindow);
            std::cout << __FUNCTION__ << ", setGraphicsWindow: " << graphicsWindow << std::endl;
        }
        else
        {
            return false;
        }
    }

    session::setGraphicsThreadId(GetCurrentThreadId());

    graphics_.reset(new D3d9Graphics());
    graphicsInit_ = graphics_->initGraphics(device, hDestWindowOverride, isD9Ex);
    if (!graphicsInit_)
    {
        graphics_.reset(nullptr);
        return false;
    }

    session::setGraphicsActive(true);
    session::setIsWindowed(graphics_->isWindowed());
    return true;
}

void D3d9Hook::uninitGraphics(IDirect3DDevice9* device)
{
    if (graphicsInit_)
    {
        graphics_->uninitGraphics(device);
        graphicsInit_ = false;

        session::setGraphicsActive(false);
    }
}

void D3d9Hook::freeGraphics()
{
    if (graphicsInit_)
    {
        graphics_->freeGraphics();
        graphicsInit_ = false;

        session::setGraphicsActive(false);
    }
}
