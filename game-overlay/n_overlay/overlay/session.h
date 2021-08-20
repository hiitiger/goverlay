#pragma once

namespace overlay_game
{
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
            {"d3d9Dll", std::to_string((std::uint64_t)d3d9Dll)},
        };
    }
};

struct DxgiHookInfo
{
    HMODULE dxgiDll = nullptr;
    HMODULE d3d10Dll = nullptr;
    HMODULE d3d11Dll = nullptr;
    HMODULE d3d12Dll = nullptr;

    bool presentHooked = false;
    bool present1Hooked = false;
    bool resizeBufferHooked = false;
    bool resizeTargetHooked = false;

    std::map<std::string, std::string> toMap() const
    {
        return {
            {"type", "DXGI"},
            {"dxgiDll", std::to_string((std::uint64_t)dxgiDll)},
            {"d3d10Dll", std::to_string((std::uint64_t)d3d10Dll)},
            {"d3d11Dll", std::to_string((std::uint64_t)d3d11Dll)},
            {"d3d12Dll", std::to_string((std::uint64_t)d3d12Dll)},
        };
    }
};

} // namespace overlay_game

class D3d9Hook;
class DXGIHook;
class InputHook;

namespace session
{
HMODULE loadModuleD3dCompiler47();
HMODULE loadD3dx9();


overlay_game::D3d9HookInfo &d3d9HookInfo();
overlay_game::DxgiHookInfo &dxgiHookInfo();

D3d9Hook *d3d9Hook();
DXGIHook *dxgiHook();

bool d3d9Hooked();
bool dxgiHooked();

bool tryD3d9Hook();
bool tryDxgiHook();

void clearD3d9Hook();
void clearDxgiHook();

InputHook *inputHook();

bool inputHooked();
bool tryInputHook();

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

void setGraphicsActive(bool active);
bool graphicsActive();

void setIsWindowed(bool windowed);
bool isWindowed();

void setOverlayConnected(bool restarted);
bool overlayConnected();

void setOverlayEnabled(bool v);
bool overlayEnabled();

void setOverlayVisible(bool v);
bool overlayVisible();

bool hasIGO();
void checkIGO();

} // namespace session
