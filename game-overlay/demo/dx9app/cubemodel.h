#pragma once
class CubeModel
{
public:
    CubeModel();
    ~CubeModel();

    void setupModel(Windows::ComPtr<IDirect3DDevice9>);
    void release();

    void render(D3DXMATRIX* world, bool RenderBoundingSphere);

private:
    Windows::ComPtr<IDirect3DDevice9> _device;

    Windows::ComPtr<ID3DXMesh> _mesh;

    std::vector<D3DMATERIAL9>       _mtrls;
    std::vector<IDirect3DTexture9*> _textures;
    Windows::ComPtr<ID3DXMesh> _sphereMesh ;
    Windows::ComPtr<ID3DXMesh> _boxMesh ;
};

struct Vertex{
    Vertex(){};
    Vertex(float x, float y, float z, float nx, float ny, float nz, float u, float v)
        :_x(x), _y(y), _z(z)
        , _nx(nx), _ny(ny), _nz(nz)
        , _u(u), _v(v)
    {
    }


    float _x, _y, _z;
    float _nx, _ny, _nz;
    float _u, _v;
    static const DWORD FVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1;
};