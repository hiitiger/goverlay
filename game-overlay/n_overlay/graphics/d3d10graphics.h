#pragma once

#include "dxgigraphics.h"


class D3d10Graphics : public DxgiGraphics
{
    Windows::ComPtr<IDXGISwapChain> swap_;

public:
    D3d10Graphics();
    ~D3d10Graphics();

    bool initGraphics(IDXGISwapChain* swap) override;
    void uninitGraphics(IDXGISwapChain* swap) override;
    void freeGraphics() override;

    void beforePresent(IDXGISwapChain* swap) override;
    void afterPresent(IDXGISwapChain* swap) override;


    bool initGraphicsContext(IDXGISwapChain *swap) override;
};