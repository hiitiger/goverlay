#include "stable.h"
#include "basicdrawapp.h"

namespace
{
    struct Vertex
    {
        XMFLOAT3 position;
        XMFLOAT4 color;;
    };

}


BasicDrawApp::BasicDrawApp()
{

}

bool BasicDrawApp::LoadContent()
{
    mCamera.SetPosition(0.0f, 2.0f, -15.0f);

    if(!buildFx())
    {
        return false;
    }

    if (!loadModel())
    {
        return false;
    }

    D3D11_RASTERIZER_DESC rsDesc;
    ZeroMemory(&rsDesc, sizeof(rsDesc));
    rsDesc.FillMode = D3D11_FILL_WIREFRAME;		//WireFrame
    rsDesc.CullMode = D3D11_CULL_BACK;
    rsDesc.FrontCounterClockwise = false;
    rsDesc.DepthClipEnable = true;
    md3dDevice->CreateRasterizerState(&rsDesc, rsState.resetAndGetPointerAddress());

    return true;
}

void BasicDrawApp::UnloadContent()
{
    rsState = nullptr;

    pEffect = nullptr;
    pWorldViewProj = nullptr;

    pVBuffer = nullptr;
    pIBuffer = nullptr;
}

void BasicDrawApp::UpdateScene(float timeDelta)
{
    worldMatrix = XMMatrixIdentity();
    viewMatrix = XMMatrixIdentity();
    projectionMatrix = XMMatrixIdentity();

    if (::GetAsyncKeyState('W') & 0x8000)
        mCamera.Walk(4.0f * timeDelta);

    if (::GetAsyncKeyState('S') & 0x8000)
        mCamera.Walk(-4.0f * timeDelta);

    if (::GetAsyncKeyState('A') & 0x8000)
        mCamera.Strafe(-4.0f * timeDelta);

    if (::GetAsyncKeyState('D') & 0x8000)
        mCamera.Strafe(4.0f * timeDelta);

    if (::GetAsyncKeyState(VK_UP) & 0x8000)
        mCamera.Pitch(1.0f * timeDelta);

    if (::GetAsyncKeyState(VK_DOWN) & 0x8000)
        mCamera.Pitch(-1.0f * timeDelta);

    if (::GetAsyncKeyState('N') & 0x8000)
        mCamera.RotateY(1.0f * timeDelta);

    if (::GetAsyncKeyState('M') & 0x8000)
        mCamera.RotateY(-1.0f * timeDelta);

   
    XMMATRIX rx, ry, rz;

    static float x = 0.f;
    rx = XMMatrixRotationX(x);

    if (GetAsyncKeyState('X') & 0x8000)
    {
        x += timeDelta;
    }

    if (x > D3DX_PI * 2)
    {
        x = 0.f;
    }

    static float y = 0.f;
    ry = XMMatrixRotationY(y);

    if (GetAsyncKeyState('Y') & 0x8000)
    {
        y += timeDelta;
    }

    if (y > D3DX_PI * 2)
    {
        y = 0.f;
    }


    static float z = 0.f;
    rz = XMMatrixRotationZ(z/*D3DX_PI / 4.0f*/);

    if (GetAsyncKeyState('Z') & 0x8000)
    {
        z += timeDelta;
    }
    if (z > D3DX_PI * 2)
    {
        z = 0.f;
    }

    worldMatrix = rz * ry * rx;

    mCamera.UpdateViewMatrix();
    viewMatrix = mCamera.View();
    projectionMatrix = mCamera.Proj();
}

void BasicDrawApp::DrawScene()
{
    md3dContext->ClearRenderTargetView(mRenderTargetView, D3DXCOLOR(Convert::ToXmColor(Colors::LightSteelBlue)));
    md3dContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

    if (GetAsyncKeyState('P')&0x8000)
    {
        md3dContext->RSSetState(rsState);
    }
    else
    {
        md3dContext->RSSetState(nullptr);
    }


    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    md3dContext->IASetVertexBuffers(0, 1, pVBuffer.getPointerAdress(), &stride, &offset);
    md3dContext->IASetIndexBuffer(pIBuffer, DXGI_FORMAT_R32_UINT, 0);
    md3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    md3dContext->IASetInputLayout(pLayout);


    XMMATRIX worldViewProj = worldMatrix*viewMatrix*projectionMatrix;


    ID3DX11EffectTechnique *tech = pEffect->GetTechniqueByName("BasicDraw");
    D3DX11_TECHNIQUE_DESC techDesc;
    tech->GetDesc(&techDesc);
    for (UINT i = 0; i < techDesc.Passes; ++i)
    {

        pWorldViewProj->SetMatrix(reinterpret_cast<float*>(&worldViewProj));
        tech->GetPassByIndex(i)->Apply(0, md3dContext);

        md3dContext->DrawIndexed(mGridIndexCount, mGridIndexOffset, mGridVertexOffset);
        md3dContext->DrawIndexed(mBoxIndexCount, mBoxIndexOffset, mBoxVertexOffset);

        {
            XMMATRIX cylinderWorldMaxtrix = XMMatrixTranslation(0.0f, 4.0f, 0.0f);

            worldViewProj = cylinderWorldMaxtrix*viewMatrix*projectionMatrix;

            pWorldViewProj->SetMatrix(reinterpret_cast<float*>(&worldViewProj));
            tech->GetPassByIndex(i)->Apply(0, md3dContext);
            md3dContext->DrawIndexed(mCylinderIndexCount, mCylinderIndexOffset, mCylinderVertexOffset);
        }


        {
            XMMATRIX sphereWorldMaxtrix = XMMatrixTranslation(4.0f, 4.0f, -4.0f);

            worldViewProj = sphereWorldMaxtrix*viewMatrix*projectionMatrix;

            pWorldViewProj->SetMatrix(reinterpret_cast<float*>(&worldViewProj));
            tech->GetPassByIndex(i)->Apply(0, md3dContext);
            md3dContext->DrawIndexed(mSphereIndexCount, mSphereIndexOffset, mSphereVertexOffset);
        }
    }
}

bool BasicDrawApp::buildFx()
{
    Windows::ComPtr<ID3DBlob> shader = nullptr;
    Windows::ComPtr<ID3DBlob> errorBlob;
    std::string error;

    D3DX11CompileFromFileW(L"fx/basicdraw.fx", 0, 0, 0, "fx_5_0", 0, 0, 0, shader.resetAndGetPointerAddress(), errorBlob.resetAndGetPointerAddress(), 0);
    if (errorBlob){
        error = std::string((char*)errorBlob->GetBufferPointer());
        return false;
    }

    D3DX11CreateEffectFromMemory(shader->GetBufferPointer(), shader->GetBufferSize(), 0, md3dDevice, pEffect.resetAndGetPointerAddress());
    // create the input layout object
    D3D11_INPUT_ELEMENT_DESC ied[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    D3DX11_PASS_DESC passDesc = { 0 };
    pEffect->GetTechniqueByName("BasicDraw")->GetPassByIndex(0)->GetDesc(&passDesc);
    md3dDevice->CreateInputLayout(ied, 2, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, pLayout.resetAndGetPointerAddress());

    pWorldViewProj = pEffect->GetVariableByName("g_worldViewProj")->AsMatrix();
   
    return true;
}

bool BasicDrawApp::loadModel()
{
    srand((unsigned int)time(nullptr));
    GeometryGenerator::MeshData box;
    GeometryGenerator::MeshData grid;
    GeometryGenerator::MeshData cylinder;
    GeometryGenerator::MeshData sphere;

    GeometryGenerator geoGen;
    geoGen.CreateBox(1.0f, 1.0f, 1.0f, box);
    geoGen.CreateGrid(20.0f, 30.0f, 60, 40, grid);
    geoGen.CreateCylinder(2.0f, 1.0f, 2.0f, 5, 10, cylinder);
    geoGen.createSphere(2.0f, 10, 10, sphere);

   // Cache the vertex offsets to each object in the concatenated vertex buffer.
    mBoxVertexOffset = 0;
    mGridVertexOffset = box.vertices.size();
    mCylinderVertexOffset = mGridVertexOffset + grid.vertices.size();
    mSphereVertexOffset = mCylinderVertexOffset + cylinder.vertices.size();

    // Cache the index count of each object.
    mBoxIndexCount = box.indices.size();
    mGridIndexCount = grid.indices.size();
    mCylinderIndexCount = cylinder.indices.size();
    mSphereIndexCount = sphere.indices.size();

    // Cache the starting index for each object in the concatenated index buffer.
    mBoxIndexOffset = 0;
    mGridIndexOffset = mBoxIndexCount;
    mCylinderIndexOffset = mGridIndexOffset + mGridIndexCount;
    mSphereIndexOffset = mCylinderIndexOffset + mCylinderIndexCount;

    UINT totalVertexCount =
        box.vertices.size() + grid.vertices.size() + cylinder.vertices.size() + sphere.vertices.size();

    UINT totalIndexCount =
        mBoxIndexCount + mGridIndexCount + mCylinderIndexCount + mSphereIndexCount;

    std::vector<Vertex> vertices(totalVertexCount);

    UINT k = 0;
    XMFLOAT4 color(1.0f, 0.0f, 0.0f, 1.0f);

    for (size_t i = 0; i < box.vertices.size(); ++i, ++k)
    {
        float colorStep = 1.0f / box.vertices.size();

        vertices[k].position = box.vertices[i].position;
        vertices[k].color = color;
        color.x = (rand() % 255) / 255.0f;
        color.y = (rand() % 255) / 255.0f;
        color.z = (rand() % 255) / 255.0f;
    }

    color = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);

    for (size_t i = 0; i < grid.vertices.size(); ++i, ++k)
    {
        float colorStep = 1.0f / grid.vertices.size();

        vertices[k].position = grid.vertices[i].position;
        vertices[k].color = color;
        color.x = (rand() % 255) / 255.0f;
        color.y = (rand() % 255) / 255.0f;
        color.z = (rand() % 255) / 255.0f;
    }

    color = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);

    for (size_t i = 0; i < cylinder.vertices.size(); ++i, ++k)
    {
        float colorStep = 1.0f / cylinder.vertices.size();

        vertices[k].position = cylinder.vertices[i].position;
        vertices[k].color = color;
        color.x = (rand() % 255) / 255.0f;
        color.y = (rand() % 255) / 255.0f;
        color.z = (rand() % 255) / 255.0f;
    }

    color = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
    for (size_t i = 0; i < sphere.vertices.size(); ++i, ++k)
    {
        float colorStep = 1.0f / sphere.vertices.size();

        vertices[k].position = sphere.vertices[i].position;
        vertices[k].color = color;
        color.x = (rand() % 255) / 255.0f;
        color.y = (rand() % 255) / 255.0f;
        color.z = (rand() % 255) / 255.0f;
    }

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(Vertex) * totalVertexCount;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = &vertices[0];
    HRERROR(md3dDevice->CreateBuffer(&vbd, &vinitData, pVBuffer.resetAndGetPointerAddress()));

    std::vector<UINT> indices;
    indices.insert(indices.end(), box.indices.begin(), box.indices.end());
    indices.insert(indices.end(), grid.indices.begin(), grid.indices.end());
    indices.insert(indices.end(), cylinder.indices.begin(), cylinder.indices.end());
    indices.insert(indices.end(), sphere.indices.begin(), sphere.indices.end());

    D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof(UINT) * totalIndexCount;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA iinitData;
    iinitData.pSysMem = &indices[0];
    HRERROR(md3dDevice->CreateBuffer(&ibd, &iinitData, pIBuffer.resetAndGetPointerAddress()));

    return true;
}

