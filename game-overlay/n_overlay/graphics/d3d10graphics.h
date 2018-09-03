#pragma once



class D3d10Graphics : public DxgiGraphics
{
    Windows::ComPtr<IDXGISwapChain> swap_;

public:
    D3d10Graphics();
    ~D3d10Graphics();

    bool initGraphics(IDXGISwapChain* swap);
    void uninitGraphics(IDXGISwapChain* swap);
    void freeGraphics();

    void beforePresent(IDXGISwapChain* swap);
    void afterPresent(IDXGISwapChain* swap);

private:
};