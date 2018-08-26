#include "d3d9graphics.h"

D3d9Graphics::D3d9Graphics()
{

}

D3d9Graphics::~D3d9Graphics()
{
    freeGraphics();
}

bool D3d9Graphics::initGraphics(IDirect3DDevice9* device, HWND hDestWindowOverride, bool isD9Ex)
{
    DAssert(!device_);

    bool success = false;

    Windows::ComPtr<IDirect3DDevice9Ex> deviceEx;
    if (isD9Ex)
    {
        device->QueryInterface(__uuidof(IDirect3DDevice9Ex), (void**)deviceEx.resetAndGetPointerAddress());
    }


    if (!success)
    {
        freeGraphics();
    }

    return success;
}

void D3d9Graphics::uninitGraphics(IDirect3DDevice9* device)
{
    if (device != device)
    {
        return;
    }
}

void D3d9Graphics::freeGraphics()
{

}

void D3d9Graphics::beforePresent(IDirect3DDevice9* device)
{
    if (device != device)
    {
        return;
    }
}

void D3d9Graphics::afterPresent(IDirect3DDevice9* device)
{
    if (device != device)
    {
        return;
    }
}
