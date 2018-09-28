#pragma once

#include "cubemodel.h"


class D9Graphics
{
public:
    D9Graphics(void);
    ~D9Graphics(void);

    bool initD3d9(HWND window);
    void shutdown();

    void resize(int w, int h);

    void setup();

    void render(float timeDelta);

private:
    HWND _window;
    int _width;
    int _height;

    Windows::ComPtr<IDirect3D9> _d3d9;
    Windows::ComPtr<IDirect3DDevice9> _device;
    Windows::ComPtr<IDirect3DTexture9> _texRT;

    CubeModel _model;
};
