#include "stable.h"

#include "basicstencilapp.h"


namespace
{
    struct Vertex
    {
        Vertex(){ ZeroMemory(this, sizeof(Vertex)); }
        Vertex(float x, float y, float z, float nx, float ny, float nz, float u, float v)
            :position(x, y, z)
            , normal(nx, ny, nz)
            , tex(u, v)
        {
        }
        XMFLOAT3 position;
        XMFLOAT3 normal;
        XMFLOAT2 tex;
    };

}

BasicStencilApp::BasicStencilApp()
{
    modelMaterial.ambient = Convert::ToXmFloat4(Colors::Silver);
    modelMaterial.diffuse = XMFLOAT4(0.2f, 0.4f, 0.6f, 0.8f);
    modelMaterial.specular = XMFLOAT4(0.9f, 0.9f, 0.9f, 16.0f);

    floorMtrl.ambient = Convert::ToXmFloat4(Colors::White);
    floorMtrl.diffuse = Convert::ToXmFloat4(Colors::White);
    floorMtrl.specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 2.0f);

    wallMtrl.ambient = Convert::ToXmFloat4(Colors::White);
    wallMtrl.diffuse = Convert::ToXmFloat4(Colors::White);
    wallMtrl.specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 2.0f);

    mirrorMtrl.ambient = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
    mirrorMtrl.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.6f);
    mirrorMtrl.specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);

    shadowMat.ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    shadowMat.diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.5f);
    shadowMat.specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 16.0f);

    mDirectionLight.ambient = Convert::ToXmFloat4(Colors::Silver);
    mDirectionLight.diffuse = XMFLOAT4(0.1f, 0.1f, 0.1f, 0.1f);
    mDirectionLight.specular = XMFLOAT4(0.9f, 0.4f, 0.4f, 1.f);
    mDirectionLight.direction = XMFLOAT3(0.0f, -0.707f, -0.707f);
}

bool BasicStencilApp::LoadContent()
{
    mCamera.SetPosition(0.0f, 2.0f, -10.0f);

    if (!initState())
    {
        return false;
    }

    if (!buildFx())
    {
        return false;
    }

    if (!loadModel())
    {
        return false;
    }

    if (!loadResource())
    {
        return false;
    }

    return true;
}

void BasicStencilApp::UnloadContent()
{
    pEffect = nullptr;
    pLayout = nullptr;

}


bool BasicStencilApp::initState()
{
    D3D11_BLEND_DESC noRenderTargetWritesDesc = { 0 };
    noRenderTargetWritesDesc.AlphaToCoverageEnable = false;
    noRenderTargetWritesDesc.IndependentBlendEnable = false;

    noRenderTargetWritesDesc.RenderTarget[0].BlendEnable = false;
    noRenderTargetWritesDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
    noRenderTargetWritesDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
    noRenderTargetWritesDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    noRenderTargetWritesDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    noRenderTargetWritesDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    noRenderTargetWritesDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    noRenderTargetWritesDesc.RenderTarget[0].RenderTargetWriteMask = 0;

    HRERROR(md3dDevice->CreateBlendState(&noRenderTargetWritesDesc, pNoRenderTargetWritesBS.resetAndGetPointerAddress()));

    D3D11_DEPTH_STENCIL_DESC mirrorDesc;
    mirrorDesc.DepthEnable = true;
    mirrorDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    mirrorDesc.DepthFunc = D3D11_COMPARISON_LESS;
    mirrorDesc.StencilEnable = true;
    mirrorDesc.StencilReadMask = 0xff;
    mirrorDesc.StencilWriteMask = 0xff;

    mirrorDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    mirrorDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    mirrorDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
    mirrorDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    // We are not rendering backfacing polygons, so these settings do not matter.
    mirrorDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    mirrorDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    mirrorDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
    mirrorDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    HRERROR(md3dDevice->CreateDepthStencilState(&mirrorDesc, pMarkMirrorDSS.resetAndGetPointerAddress()));

    D3D11_DEPTH_STENCIL_DESC drawReflectionDesc;
    drawReflectionDesc.DepthEnable = true;
    drawReflectionDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    drawReflectionDesc.DepthFunc = D3D11_COMPARISON_LESS;
    drawReflectionDesc.StencilEnable = true;
    drawReflectionDesc.StencilReadMask = 0xff;
    drawReflectionDesc.StencilWriteMask = 0xff;

    drawReflectionDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    drawReflectionDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    drawReflectionDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    drawReflectionDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;

    // We are not rendering backfacing polygons, so these settings do not matter.
    drawReflectionDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    drawReflectionDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    drawReflectionDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    drawReflectionDesc.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;

    HRERROR(md3dDevice->CreateDepthStencilState(&drawReflectionDesc, pDrawReflectionDSS.resetAndGetPointerAddress()));

    D3D11_DEPTH_STENCIL_DESC noDoubleBlendDesc;
    noDoubleBlendDesc.DepthEnable = true;
    noDoubleBlendDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    noDoubleBlendDesc.DepthFunc = D3D11_COMPARISON_LESS;
    noDoubleBlendDesc.StencilEnable = true;
    noDoubleBlendDesc.StencilReadMask = 0xff;
    noDoubleBlendDesc.StencilWriteMask = 0xff;

    noDoubleBlendDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    noDoubleBlendDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    noDoubleBlendDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_INCR;
    noDoubleBlendDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;

    // We are not rendering backfacing polygons, so these settings do not matter.
    drawReflectionDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    drawReflectionDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    drawReflectionDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_INCR;
    drawReflectionDesc.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;

    HRERROR(md3dDevice->CreateDepthStencilState(&drawReflectionDesc, pNoDoubleDDS.resetAndGetPointerAddress()));


    // NoCullRS
    D3D11_RASTERIZER_DESC noCullDesc;
    ZeroMemory(&noCullDesc, sizeof(D3D11_RASTERIZER_DESC));
    noCullDesc.FillMode = D3D11_FILL_SOLID;
    noCullDesc.CullMode = D3D11_CULL_NONE;
    noCullDesc.FrontCounterClockwise = false;
    noCullDesc.DepthClipEnable = true;

    HRERROR(md3dDevice->CreateRasterizerState(&noCullDesc, pNoCullRS.resetAndGetPointerAddress()));

    // CullClockwiseRS
    D3D11_RASTERIZER_DESC cullClockwiseDesc;
    ZeroMemory(&cullClockwiseDesc, sizeof(D3D11_RASTERIZER_DESC));
    cullClockwiseDesc.FillMode = D3D11_FILL_SOLID;
    cullClockwiseDesc.CullMode = D3D11_CULL_BACK;
    cullClockwiseDesc.FrontCounterClockwise = true;
    cullClockwiseDesc.DepthClipEnable = true;

    HRERROR(md3dDevice->CreateRasterizerState(&cullClockwiseDesc, pCullClockwiseRS.resetAndGetPointerAddress()));

    return true;
}

bool BasicStencilApp::buildFx()
{
    Windows::ComPtr<ID3DBlob> shader = nullptr;
    Windows::ComPtr<ID3DBlob> errorBlob;
    std::string error;

    D3DX11CompileFromFileW(L"fx/basicstencil.fx", 0, 0, 0, "fx_5_0", 0, 0, 0, shader.resetAndGetPointerAddress(), errorBlob.resetAndGetPointerAddress(), 0);
    if (errorBlob) {
        error = std::string((char*)errorBlob->GetBufferPointer());
        return false;
    }

    D3DX11CreateEffectFromMemory(shader->GetBufferPointer(), shader->GetBufferSize(), 0, md3dDevice, pEffect.resetAndGetPointerAddress());
    // create the input layout object
    D3D11_INPUT_ELEMENT_DESC ied[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    D3DX11_PASS_DESC passDesc = { 0 };
    pEffect->GetTechniqueByName("TextureDrawWithLight")->GetPassByIndex(0)->GetDesc(&passDesc);
    HRESULT hr = (md3dDevice->CreateInputLayout(ied, sizeof(ied) / sizeof(D3D11_INPUT_ELEMENT_DESC), passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, pLayout.resetAndGetPointerAddress()));
    if (FAILED(hr))
    {
        return false;
    }

    pWorldMatrix = pEffect->GetVariableByName("worldMatrix")->AsMatrix();
    pViewMatrix = pEffect->GetVariableByName("viewMatrix")->AsMatrix();
    pProjMatrix = pEffect->GetVariableByName("projectionMatrix")->AsMatrix();
    pTexTransformMatrix = pEffect->GetVariableByName("texTransformMatrix")->AsMatrix();
    pWorldInvTranspose = pEffect->GetVariableByName("worldInvTranspose")->AsMatrix();

    pShaderTexture = pEffect->GetVariableByName("image")->AsShaderResource();

    pCameraPosition = pEffect->GetVariableByName("cameraPosition");

    pMaterial = pEffect->GetVariableByName("gMaterial");

    pDirectionLight = pEffect->GetVariableByName("gDirLight");

    pPointLight = pEffect->GetVariableByName("gPointLight");

    pSpotLight = pEffect->GetVariableByName("gSpotLight");

    return true;
}

bool BasicStencilApp::loadModel()
{
    std::vector<Vertex> v(24);
    //
    // Create and specify geometry.  For this sample we draw a floor
    // and a wall with a mirror on it.  We put the floor, wall, and
    // mirror geometry in one vertex buffer.
    //
    //   |----|----|----|
    //   |Wall|Mirr|Wall|
    //   |    | or |    |
    //   /--------------/
    //  /   Floor      /
    // /--------------/

    // floor
    //   1 /------------2
    //    /   _   -
    //  0/-
    v[0] = Vertex(-7.5f, 0.0f, -10.0f, 0.0f, 1.0f, 0.0f, 0.0f, 4.0f);
    v[1] = Vertex(-7.5f, 0.0f, 10.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f);
    v[2] = Vertex(7.5f, 0.0f, 10.0f, 0.0f, 1.0f, 0.0f, 4.0f, 0.0f);

    v[3] = Vertex(-7.5f, 0.0f, -10.0f, 0.0f, 1.0f, 0.0f, 0.0f, 4.0f);
    v[4] = Vertex(7.5f, 0.0f, 10.0f, 0.0f, 1.0f, 0.0f, 4.0f, 0.0f);
    v[5] = Vertex(7.5f, 0.0f, -10.0f, 0.0f, 1.0f, 0.0f, 4.0f, 4.0f);

    // wall
    v[6] = Vertex(-7.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
    v[7] = Vertex(-7.5f, 5.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
    v[8] = Vertex(-2.5f, 5.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);

    v[9] = Vertex(-7.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
    v[10] = Vertex(-2.5f, 5.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
    v[11] = Vertex(-2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);

    // Note: We leave gap in middle of walls for mirror
    v[12] = Vertex(2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
    v[13] = Vertex(2.5f, 5.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
    v[14] = Vertex(7.5f, 5.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);

    v[15] = Vertex(2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
    v[16] = Vertex(7.5f, 5.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
    v[17] = Vertex(7.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);

    // mirror

    v[18] = Vertex(-2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
    v[19] = Vertex(-2.5f, 5.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
    v[20] = Vertex(2.5f, 5.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);

    v[21] = Vertex(-2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
    v[22] = Vertex(2.5f, 5.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
    v[23] = Vertex(2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(Vertex) * v.size();
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = &v[0];
    HRERROR(md3dDevice->CreateBuffer(&vbd, &vinitData, pVBuffer.resetAndGetPointerAddress()));

    GeometryGenerator::MeshData sphere;
    GeometryGenerator geoGen;
    geoGen.createSphere(1.0f, 100, 100, sphere);

    std::vector<Vertex> vertices(sphere.vertices.size());

    UINT k = 0;
    for (size_t i = 0; i < sphere.vertices.size(); ++i, ++k)
    {
        vertices[k].position = sphere.vertices[i].position;
        vertices[k].normal = sphere.vertices[i].normal;
        vertices[k].tex = sphere.vertices[i].tex;
    }

    D3D11_BUFFER_DESC modelvbd;
    modelvbd.Usage = D3D11_USAGE_IMMUTABLE;
    modelvbd.ByteWidth = sizeof(Vertex) * vertices.size();
    modelvbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    modelvbd.CPUAccessFlags = 0;
    modelvbd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA modelvinitData;
    modelvinitData.pSysMem = &vertices[0];
    HRERROR(md3dDevice->CreateBuffer(&modelvbd, &modelvinitData, pModelVBuffer.resetAndGetPointerAddress()));

    std::vector<UINT> modelindices;
    modelindices.insert(modelindices.end(), sphere.indices.begin(), sphere.indices.end());
    modelIndexCount = modelindices.size();

    D3D11_BUFFER_DESC modelibd;
    modelibd.Usage = D3D11_USAGE_IMMUTABLE;
    modelibd.ByteWidth = sizeof(UINT) * modelindices.size();
    modelibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    modelibd.CPUAccessFlags = 0;
    modelibd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA modeliinitData;
    modeliinitData.pSysMem = &modelindices[0];
    HRERROR(md3dDevice->CreateBuffer(&modelibd, &modeliinitData, pModelIBuffer.resetAndGetPointerAddress()));

    return true;
}


bool BasicStencilApp::loadResource()
{
    D3DX11_IMAGE_LOAD_INFO loadInfo;
    ZeroMemory(&loadInfo, sizeof(D3DX11_IMAGE_LOAD_INFO));
    loadInfo.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    loadInfo.Format = DXGI_FORMAT_BC2_UNORM;
    loadInfo.MipLevels = D3DX11_DEFAULT;
    loadInfo.MipFilter = D3DX11_FILTER_LINEAR;

    HRERROR(D3DX11CreateShaderResourceViewFromFile(md3dDevice, L"res/water1.dds", &loadInfo, NULL, m_modeltexture.resetAndGetPointerAddress(), NULL));
    HRERROR(D3DX11CreateShaderResourceViewFromFile(md3dDevice, L"res/checkboard.dds", &loadInfo, NULL, m_floortexture.resetAndGetPointerAddress(), NULL));
    HRERROR(D3DX11CreateShaderResourceViewFromFile(md3dDevice, L"res/ice.dds", &loadInfo, NULL, m_mirrortexture.resetAndGetPointerAddress(), NULL));
    HRERROR(D3DX11CreateShaderResourceViewFromFile(md3dDevice, L"res/brick01.dds", &loadInfo, NULL, m_walltexture.resetAndGetPointerAddress(), NULL));

    return true;
}

void BasicStencilApp::UpdateScene(float timeDelta)
{
    worldMatrix = XMMatrixIdentity();
    viewMatrix = XMMatrixIdentity();
    projectionMatrix = XMMatrixIdentity();
    waterTexTransformMaxtrix = XMMatrixIdentity();
    modelWorldMatrix = XMMatrixTranslation(0.0f, 3.0f, -2.0f);

    float speed = g_walkspeed * ((GetAsyncKeyState(VK_SHIFT) * 0x8000) ? 3.0f : 1.0f);

    if (::GetAsyncKeyState('W') & 0x8000)
        mCamera.Walk(speed * timeDelta);

    if (::GetAsyncKeyState('S') & 0x8000)
        mCamera.Walk(-speed * timeDelta);

    if (::GetAsyncKeyState('A') & 0x8000)
        mCamera.Strafe(-speed * timeDelta);

    if (::GetAsyncKeyState('D') & 0x8000)
        mCamera.Strafe(speed * timeDelta);

    if (::GetAsyncKeyState('R') & 0x8000)
        mCamera.Fly(speed * timeDelta);

    if (::GetAsyncKeyState('F') & 0x8000)
        mCamera.Fly(-speed * timeDelta);

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
    static float theta = 0;
    theta += timeDelta;
    if (theta >= XM_2PI)
    {
        theta = 0;
    }


    float scaleModel = 1.0f + abs(sinf(theta));
    XMMATRIX modelScaleMatrix = XMMatrixScaling(scaleModel, scaleModel, scaleModel);
    modelWorldMatrix = modelScaleMatrix * modelWorldMatrix;

    static float waterTexTranlateX = 0.0f;
    static float waterTexTranlateY = 0.0f;

    waterTexTranlateX += timeDelta;
    waterTexTranlateY += timeDelta;

    waterTexTransformMaxtrix = XMMatrixTranslation(waterTexTranlateX * 0.01f, waterTexTranlateY * 0.1f, 0.0);

    mDirectionLight.direction = XMFLOAT3((sinf(theta)), -1.0f, (cosf(theta)));
    XMStoreFloat3(&mDirectionLight.direction, XMVector3Normalize(XMLoadFloat3(&mDirectionLight.direction)));

    mCamera.UpdateViewMatrix();

    viewMatrix = mCamera.View();

    float  fieldOfView = (float)XM_PI / ((GetAsyncKeyState('V') & 0x8000) ? 2.0f : 4.0f);
    float screenAspect = (float)mClientWidth / (float)mClientHeight;
    mCamera.SetLens(fieldOfView, screenAspect, 1.0f, 1000.0f);

    projectionMatrix = mCamera.Proj();

    setShaderParams();
}

void BasicStencilApp::setShaderParams()
{
    pWorldMatrix->SetMatrix(reinterpret_cast<float*>(&worldMatrix));
    pViewMatrix->SetMatrix(reinterpret_cast<float*>(&viewMatrix));
    pProjMatrix->SetMatrix(reinterpret_cast<float*>(&projectionMatrix));

    pDirectionLight->SetRawValue(&mDirectionLight, 0, sizeof(mDirectionLight));

    XMFLOAT3 camPos = mCamera.GetPosition();
    pCameraPosition->SetRawValue(&camPos, 0, sizeof(camPos));
}

void BasicStencilApp::DrawScene()
{
    md3dContext->ClearRenderTargetView(mRenderTargetView, D3DXCOLOR(Convert::ToXmColor(Colors::LightSteelBlue)));
    md3dContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    //1. 绘制普通场景
    _drawScene();

    _drawMirror();
}

void BasicStencilApp::_drawScene()
{
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    md3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    md3dContext->IASetInputLayout(pLayout);

    std::string techName = GetAsyncKeyState('P') & 0x8000 ? "LightingDraw" :  "TextureDrawWithLight";

    ID3DX11EffectTechnique *tech = pEffect->GetTechniqueByName(techName.c_str());
    D3DX11_TECHNIQUE_DESC techDesc;
    tech->GetDesc(&techDesc);

    enableAlpha();

    for (UINT i = 0; i < techDesc.Passes; ++i)
    {
        md3dContext->IASetVertexBuffers(0, 1, pVBuffer.getPointerAdress(), &stride, &offset);
        XMMATRIX indentityMatrix = XMMatrixIdentity();
        //floor
        {
            pTexTransformMatrix->SetMatrix(reinterpret_cast<float*>(&indentityMatrix));

            pWorldMatrix->SetMatrix((const float*)&worldMatrix);
            XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(worldMatrix);
            pWorldInvTranspose->SetMatrix((const float*)&worldInvTranspose);

            pMaterial->SetRawValue(&floorMtrl, 0, sizeof(floorMtrl));
            pShaderTexture->SetResource(m_floortexture);

            tech->GetPassByIndex(i)->Apply(0, md3dContext);
            md3dContext->Draw(6, 0);
        }

        //wall
        {
            pWorldMatrix->SetMatrix((const float*)&worldMatrix);
            XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(worldMatrix);
            pWorldInvTranspose->SetMatrix((const float*)&worldInvTranspose);

            pMaterial->SetRawValue(&wallMtrl, 0, sizeof(wallMtrl));
            pShaderTexture->SetResource(m_walltexture);

            tech->GetPassByIndex(i)->Apply(0, md3dContext);
            md3dContext->Draw(12, 6);
        }


        md3dContext->IASetVertexBuffers(0, 1, pModelVBuffer.getPointerAdress(), &stride, &offset);
        md3dContext->IASetIndexBuffer(pModelIBuffer, DXGI_FORMAT_R32_UINT, 0);

        {
            XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(modelWorldMatrix);

            pWorldMatrix->SetMatrix((const float*)&modelWorldMatrix);
            pWorldInvTranspose->SetMatrix((const float*)&worldInvTranspose);
            pTexTransformMatrix->SetMatrix((const float*)&waterTexTransformMaxtrix);

            pMaterial->SetRawValue(&modelMaterial, 0, sizeof(modelMaterial));
            pShaderTexture->SetResource(m_modeltexture);
            tech->GetPassByIndex(i)->Apply(0, md3dContext);
            md3dContext->DrawIndexed(modelIndexCount, 0, 0);
        }
    }
    
    restoreAlpha();
}

void BasicStencilApp::_drawMirror()
{
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    md3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    md3dContext->IASetInputLayout(pLayout);

    std::string techName = GetAsyncKeyState('P') & 0x8000 ? "LightingDraw" : "TextureDrawWithLight";

    ID3DX11EffectTechnique *tech = pEffect->GetTechniqueByName(techName.c_str());
    D3DX11_TECHNIQUE_DESC techDesc;
    tech->GetDesc(&techDesc);

    float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };

    XMMATRIX indentityMatrix = XMMatrixIdentity();

    //2. 镜子绘制到模板缓存
    //draw mirror to stencil buffer
    for (UINT i = 0; i < techDesc.Passes; ++i)
    {
        md3dContext->IASetVertexBuffers(0, 1, pVBuffer.getPointerAdress(), &stride, &offset);

        //mirror
        pTexTransformMatrix->SetMatrix((const float*)&indentityMatrix);

        pWorldMatrix->SetMatrix((const float*)&worldMatrix);
        XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(worldMatrix);
        pWorldInvTranspose->SetMatrix((const float*)&worldInvTranspose);

        pMaterial->SetRawValue(&mirrorMtrl, 0, sizeof(mirrorMtrl));
        pShaderTexture->SetResource(m_mirrortexture);

        md3dContext->OMSetBlendState(pNoRenderTargetWritesBS, blendFactor, 0xffffffff);
        md3dContext->OMSetDepthStencilState(pMarkMirrorDSS, 1);

        tech->GetPassByIndex(i)->Apply(0, md3dContext);
        md3dContext->Draw(6, 18);

        // Restore states.
        md3dContext->OMSetDepthStencilState(0, 0);
        md3dContext->OMSetBlendState(0, blendFactor, 0xffffffff);
    }

    enableAlpha();

    //3. 绘制镜中反射的物体, 进行模板测试
    //draw  reflection
    for (UINT i = 0; i < techDesc.Passes; ++i)
    {
        md3dContext->IASetVertexBuffers(0, 1, pModelVBuffer.getPointerAdress(), &stride, &offset);
        md3dContext->IASetIndexBuffer(pModelIBuffer, DXGI_FORMAT_R32_UINT, 0);

        
        XMVECTOR mirrorPlane = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f); // xy plane
        XMMATRIX R = XMMatrixReflect(mirrorPlane);
        XMMATRIX world = modelWorldMatrix * R;
        XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);

        pWorldMatrix->SetMatrix((const float*)&world);
        pWorldInvTranspose->SetMatrix((const float*)&worldInvTranspose);
        pTexTransformMatrix->SetMatrix((const float*)&waterTexTransformMaxtrix);

        pMaterial->SetRawValue(&modelMaterial, 0, sizeof(modelMaterial));
        pShaderTexture->SetResource(m_modeltexture);


        // Cache the old light directions, and reflect the light directions.
        XMFLOAT3 oldLightDirections;
        oldLightDirections = mDirectionLight.direction;
        XMVECTOR lightDir = XMLoadFloat3(&oldLightDirections);
        XMVECTOR reflectedLightDir = XMVector3TransformNormal(lightDir, R);
        XMStoreFloat3(&mDirectionLight.direction, reflectedLightDir);
        pDirectionLight->SetRawValue(&mDirectionLight, 0, sizeof(mDirectionLight));

        md3dContext->OMSetDepthStencilState(pDrawReflectionDSS, 1);
        md3dContext->RSSetState(pCullClockwiseRS);
          
        tech->GetPassByIndex(i)->Apply(0, md3dContext);
        md3dContext->DrawIndexed(modelIndexCount, 0, 0);

        // Restore states.
        md3dContext->RSSetState(0);
        md3dContext->OMSetDepthStencilState(0, 0);

        mDirectionLight.direction = oldLightDirections;
        pDirectionLight->SetRawValue(&mDirectionLight, 0, sizeof(mDirectionLight));  
    }

    //4. 绘制镜子
    //draw mirror

    for (UINT p = 0; p < techDesc.Passes; ++p)
    { 
        md3dContext->IASetVertexBuffers(0, 1, pVBuffer.getPointerAdress(), &stride, &offset);
        XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(worldMatrix);
      
        pWorldMatrix->SetMatrix((const float*)&worldMatrix);
        pWorldInvTranspose->SetMatrix((const float*)&worldInvTranspose);
        pTexTransformMatrix->SetMatrix((const float*)&indentityMatrix);

        pMaterial->SetRawValue(&mirrorMtrl, 0, sizeof(mirrorMtrl));
        pShaderTexture->SetResource(m_mirrortexture);

        tech->GetPassByIndex(p)->Apply(0, md3dContext);
        md3dContext->Draw(6, 18);
    }

    //绘制影子

    for (UINT i = 0; i < techDesc.Passes; ++i)
    {
        md3dContext->IASetVertexBuffers(0, 1, pModelVBuffer.getPointerAdress(), &stride, &offset);
        md3dContext->IASetIndexBuffer(pModelIBuffer, DXGI_FORMAT_R32_UINT, 0);

        XMVECTOR shadowPlane = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f); // xz plane
        XMVECTOR toMainLight = -XMLoadFloat3(&mDirectionLight.direction);
        XMMATRIX S = XMMatrixShadow(shadowPlane, toMainLight);
        XMMATRIX shadowOffsetY = XMMatrixTranslation(0.0f, 0.001f, 0.0f);

        XMMATRIX world = XMLoadFloat4x4((const XMFLOAT4X4*)&modelWorldMatrix)*S*shadowOffsetY;
        XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
  
        pWorldMatrix->SetMatrix((const float*)&world);
        pWorldInvTranspose->SetMatrix((const float*)&worldInvTranspose);
        pTexTransformMatrix->SetMatrix((const float*)&waterTexTransformMaxtrix);

        pMaterial->SetRawValue(&shadowMat, 0, sizeof(shadowMat));
        pShaderTexture->SetResource(m_mirrortexture);

        md3dContext->OMSetDepthStencilState(pNoDoubleDDS, 0);
        tech->GetPassByIndex(i)->Apply(0, md3dContext);
        md3dContext->DrawIndexed(modelIndexCount, 0, 0);

        md3dContext->OMSetDepthStencilState(0, 0);
    }

    restoreAlpha();
}

LRESULT BasicStencilApp::WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_KEYDOWN)
    {
        if (wParam == 'L')
        {
            m_useLighting = !m_useLighting;
        }
    }

    return __super::WindowProc(hwnd, message, wParam, lParam);
}
