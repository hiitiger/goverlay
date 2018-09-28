#include "stdafx.h"
#include "cubemodel.h"



bool ComputeBoundingSphere(ID3DXMesh* mesh, D3DUtils::BoundingSphere* sphere)
{
    HRESULT hr = 0;

    BYTE* v = 0;
    mesh->LockVertexBuffer(0, (void**)&v);

    hr = D3DXComputeBoundingSphere(
        (D3DXVECTOR3*)v,
        mesh->GetNumVertices(),
        D3DXGetFVFVertexSize(mesh->GetFVF()),
        &sphere->_center,
        &sphere->_radius);

    mesh->UnlockVertexBuffer();

    if (FAILED(hr))
        return false;

    return true;
}
bool ComputeBoundingBox(ID3DXMesh* mesh, D3DUtils::BoundingBox* box)
{
    HRESULT hr = 0;

    BYTE* v = 0;
    mesh->LockVertexBuffer(0, (void**)&v);

    hr = D3DXComputeBoundingBox(
        (D3DXVECTOR3*)v,
        mesh->GetNumVertices(),
        D3DXGetFVFVertexSize(mesh->GetFVF()),
        &box->_min,
        &box->_max);

    mesh->UnlockVertexBuffer();

    if (FAILED(hr))
        return false;

    return true;
}


CubeModel::CubeModel()
{
}


CubeModel::~CubeModel()
{
}

void CubeModel::setupModel(Windows::ComPtr<IDirect3DDevice9> device)
{
    _device = device;


    Windows::ComPtr<ID3DXBuffer> adjBuffer = 0;
    Windows::ComPtr<ID3DXBuffer>  mtrlBuffer = 0;
    DWORD        numMtrls = 0;

   HRESULT hr =  D3DXLoadMeshFromXW(
        L"bigship1.x",
        D3DXMESH_MANAGED,
        device,
        adjBuffer.resetAndGetPointerAddress(),
        mtrlBuffer.resetAndGetPointerAddress(),
        0,
        &numMtrls,
        _mesh.resetAndGetPointerAddress());

   if (FAILED(hr))
   {
       return;
   }

   if (mtrlBuffer != 0 && numMtrls != 0)
   {
       D3DXMATERIAL* mtrls = (D3DXMATERIAL*)mtrlBuffer->GetBufferPointer();

       for (int i = 0; i < numMtrls; i++)
       {
           D3DXMATERIAL mtrl = mtrls[i];
           // the MatD3D property doesn't have an ambient value set
           // when its loaded, so set it now:
           mtrl.MatD3D.Ambient = mtrl.MatD3D.Diffuse;

           // save the ith material
           _mtrls.push_back(mtrl.MatD3D);

           // check if the ith material has an associative texture
           if (mtrl.pTextureFilename != 0)
           {
               // yes, load the texture for the ith subset
               IDirect3DTexture9* tex = 0;
               D3DXCreateTextureFromFileA(
                   device,
                   mtrl.pTextureFilename,
                   &tex);

               // save the loaded texture
               _textures.push_back(tex);
           }
           else
           {
               // no texture for the ith subset
               _textures.push_back(0);
           }
       }
   }

   hr = _mesh->OptimizeInplace(
       D3DXMESHOPT_ATTRSORT |
       D3DXMESHOPT_COMPACT |
       D3DXMESHOPT_VERTEXCACHE,
       (DWORD*)adjBuffer->GetBufferPointer(),
       0, 0, 0);

   if (FAILED(hr))
   {
       return;
   }

   D3DUtils::BoundingSphere boundingSphere;
   D3DUtils::BoundingBox    boundingBox;

   ComputeBoundingSphere(_mesh, &boundingSphere);
   ComputeBoundingBox(_mesh, &boundingBox);

   D3DXCreateSphere(
       device,
       boundingSphere._radius,
       40,
       40,
       _sphereMesh.resetAndGetPointerAddress(),
       0);

   D3DXCreateBox(
       device,
       boundingBox._max.x - boundingBox._min.x,
       boundingBox._max.y - boundingBox._min.y,
       boundingBox._max.z - boundingBox._min.z,
       _boxMesh.resetAndGetPointerAddress(),
       0);

}

void CubeModel::release()
{
    _mesh = nullptr;
    _mtrls.clear();
    _textures.clear();
    _sphereMesh = nullptr;
    _boxMesh = nullptr;

    _device = nullptr;
}


void CubeModel::render(D3DXMATRIX* world, bool RenderBoundingSphere)
{
    _device->SetTransform(D3DTS_WORLD, world);

    for (int i = 0; i < _mtrls.size(); i++)
    {
        _device->SetMaterial(&_mtrls[i]);
        _device->SetTexture(0, _textures[i]);
        _mesh->DrawSubset(i);
    }

    // Draw bounding volume in blue and at 10% opacity
    D3DMATERIAL9 blue = D3DUtils::BLUE_MTRL;
    blue.Diffuse.a = 0.10f; // 10% opacity

    _device->SetMaterial(&blue);
    _device->SetTexture(0, 0); // disable texture

    _device->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
    _device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    _device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

    if (RenderBoundingSphere)
        _sphereMesh->DrawSubset(0);
    else
        _boxMesh->DrawSubset(0);




    _device->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
}
