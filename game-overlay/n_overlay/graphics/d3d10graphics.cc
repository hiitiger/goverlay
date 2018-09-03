#include "D3d10Graphics.h"

D3d10Graphics::D3d10Graphics()
{

}

D3d10Graphics::~D3d10Graphics()
{
    freeGraphics();
}

bool D3d10Graphics::initGraphics(IDXGISwapChain* swap)
{
    DAssert(!swap_);

    bool success = false;

    if (!success)
    {
        freeGraphics();
    }

    return success;
}

void D3d10Graphics::uninitGraphics(IDXGISwapChain* swap)
{
    if (swap_.get() != swap)
    {
        return;
    }

    freeGraphics();
}

void D3d10Graphics::freeGraphics()
{

}

void D3d10Graphics::beforePresent(IDXGISwapChain* swap)
{
    if (swap_.get() != swap)
    {
        return;
    }
}

void D3d10Graphics::afterPresent(IDXGISwapChain* swap)
{
    if (swap_.get() != swap)
    {
        return;
    }
}