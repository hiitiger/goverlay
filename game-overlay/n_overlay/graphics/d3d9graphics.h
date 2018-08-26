#pragma once



class D3d9Graphics
{
    Windows::ComPtr<IDirect3DDevice9> device_;

public:
    D3d9Graphics();
    ~D3d9Graphics();

    bool initGraphics(IDirect3DDevice9* device, HWND hDestWindowOverride, bool isD9Ex);
    void uninitGraphics(IDirect3DDevice9* device);
    void freeGraphics();

    void beforePresent(IDirect3DDevice9* device);
    void afterPresent(IDirect3DDevice9* device);

private:
};