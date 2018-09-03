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

    bool success = false;

    if (!success)
    {
        freeGraphics();
    }

    return success;
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

}

void D3d11Graphics::beforePresent(IDXGISwapChain* swap)
{
    if (swap_.get() != swap)
    {
        return;
    }
}

void D3d11Graphics::afterPresent(IDXGISwapChain* swap)
{
    if (swap_.get() != swap)
    {
        return;
    }
}

bool D3d11Graphics::initGraphicsContext(IDXGISwapChain *swap)
{
    return true;
}
