#pragma once


struct  D3d9HookInfo
{
    HMODULE d3d9Dll = nullptr;

    Windows::ComPtr<IDirect3DDevice9> device;
};


struct  DxgiHookInfo
{
    HMODULE dxgiDll = nullptr;
    HMODULE d3d10Dll = nullptr;
    HMODULE d3d11Dll = nullptr;
};

class D3d9Hook;
class DXGIHook;

namespace session
{

    D3d9HookInfo& d3d9HookInfo();
    DxgiHookInfo& dxgiHookInfo();

    D3d9Hook* d3d9Hook();
    DXGIHook* dxgiHook();

    bool d3d9Hooked();
    bool dxgiHooked();

    void saveD3d9Hook(std::unique_ptr<D3d9Hook>&& h);
    void saveDxgiHook(std::unique_ptr<DXGIHook>&& h);

    void clearD3d9Hook();
    void clearDxgiHook();

    bool inputHooked();
    void saveInputHook();

    void setInjectWindow(HWND window);
    HWND injectWindow();

    void setGraphicsWindow(HWND window);
    HWND graphicsWindow();

    std::uint32_t hookAppThreadId();
    void setHookAppThreadId(DWORD id);

    std::uint32_t windowThreadId();
    void setWindowThreadId(DWORD id);

    std::uint32_t graphicsThreadId();
    void setGraphicsThreadId(DWORD id);
}


enum class Threads {
    HookApp = 1,
    Graphics = 2,
    Window = 3,
};


inline bool checkThread(Threads type)
{
    std::uint32_t required = 0;
    switch (type)
    {
    case Threads::HookApp:
        required = session::hookAppThreadId();
        break;
    case Threads::Graphics:
        required = session::graphicsThreadId();
        break;
    case Threads::Window:
        required = session::windowThreadId();
        break;
    default:
        break;
    }

    return ::GetCurrentProcessId() == required;

}

#define CHECK_THREAD(type) \
do \
{\
    assert(checkThread(type));\
} while (0);
