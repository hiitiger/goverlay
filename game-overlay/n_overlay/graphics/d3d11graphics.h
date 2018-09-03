#pragma once



class D3d11Graphics : public DxgiGraphics
{
    Windows::ComPtr<IDXGISwapChain> swap_;

public:
    D3d11Graphics();
    ~D3d11Graphics();

    bool initGraphics(IDXGISwapChain* swap);
    void uninitGraphics(IDXGISwapChain* swap);
    void freeGraphics();

    void beforePresent(IDXGISwapChain* swap);
    void afterPresent(IDXGISwapChain* swap);

private:
};