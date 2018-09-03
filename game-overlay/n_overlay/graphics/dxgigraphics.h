
#pragma once


class DxgiGraphics
{
public:
    virtual ~DxgiGraphics() {}
    virtual bool initGraphics(IDXGISwapChain *swap) = 0;
    virtual void uninitGraphics(IDXGISwapChain *swap) = 0;
    virtual void freeGraphics() = 0;

    virtual void beforePresent(IDXGISwapChain *swap) = 0;
    virtual void afterPresent(IDXGISwapChain *swap) = 0;

    virtual bool initGraphicsContext(IDXGISwapChain *swap) = 0;
};