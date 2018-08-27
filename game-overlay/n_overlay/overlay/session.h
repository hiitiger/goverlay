#pragma once

struct Hotkey
{
    std::string name;
    int vkey;
    bool ctrl;
    bool shift;
    bool alt;
};

struct D3d9HookInfo
{
    bool endSceneHooked = false;
    bool presentHooked = false;
    bool presentExHooked = false;
    bool swapChainPresentHooked = false;
    bool resetHooked = false;
    bool resetExHooked = false;

    HMODULE d3d9Dll = nullptr;

    std::map<std::string, std::string> toMap() const
    {
        return {
            {"type", "D3d9"},
            {"endSceneHooked", endSceneHooked ? "true" : "false"},
            {"presentHooked", presentHooked ? "true" : "false"},
            {"presentExHooked", presentExHooked ? "true" : "false"},
            {"swapChainPresentHooked", swapChainPresentHooked ? "true" : "false"},
            {"resetHooked", resetHooked ? "true" : "false"},
            {"resetExHooked", resetExHooked ? "true" : "false"},
            {"d3d9Dll", std::to_string((std::uint32_t)d3d9Dll)},
        };
    }
};

struct DxgiHookInfo
{
    HMODULE dxgiDll = nullptr;
    HMODULE d3d10Dll = nullptr;
    HMODULE d3d11Dll = nullptr;

    std::map<std::string, std::string> toMap() const
    {
        return {
            {"type", "DXGI"},
            {"dxgiDll", std::to_string((std::uint32_t)dxgiDll)},
            {"d3d10Dll", std::to_string((std::uint32_t)d3d10Dll)},
            {"d3d11Dll", std::to_string((std::uint32_t)d3d11Dll)},
        };
    }
};

class D3d9Hook;
class DXGIHook;
class InputHook;

namespace session
{

D3d9HookInfo &d3d9HookInfo();
DxgiHookInfo &dxgiHookInfo();

D3d9Hook *d3d9Hook();
DXGIHook *dxgiHook();

bool d3d9Hooked();
bool dxgiHooked();

void saveD3d9Hook(std::unique_ptr<D3d9Hook> &&h);
void saveDxgiHook(std::unique_ptr<DXGIHook> &&h);

void clearD3d9Hook();
void clearDxgiHook();

InputHook *inputHook();

bool inputHooked();
void saveInputHook(std::unique_ptr<InputHook> &&h);

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

} // namespace session
