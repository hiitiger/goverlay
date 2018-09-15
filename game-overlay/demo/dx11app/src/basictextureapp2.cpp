#include "stable.h"

#include "BasicTextureApp2.h"


namespace
{
    struct Vertex
    {
        XMFLOAT3 position;
        XMFLOAT3 normal;
        XMFLOAT2 tex;
    };

}

float GetHillHeight(float x, float z)
{
    return 0.3f*(z*sinf(0.2f*x) + x*cosf(0.2f*z));
}
XMFLOAT3 GetHillNormal(float x, float z)
{
    // n = (-df/dx, 1, -df/dz)
    XMFLOAT3 n(
        -0.03f*z*cosf(0.2f*x) - 0.3f*cosf(0.2f*z),
        1.0f,
        -0.3f*sinf(0.2f*x) + 0.03f*x*sinf(0.2f*z));

    XMVECTOR unitNormal = XMVector3Normalize(XMLoadFloat3(&n));
    XMStoreFloat3(&n, unitNormal);

    return n;
}


BasicTextureApp2::BasicTextureApp2()
{
    groundMaterial.ambient = Convert::ToXmFloat4(Colors::Silver);
    groundMaterial.diffuse = XMFLOAT4(0.7f, 0.85f, 0.7f, 1.0f);
    groundMaterial.specular = XMFLOAT4(1.0f, 0.8f, 0.8f, 4.0f);

    waterMaterial.ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    waterMaterial.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f);
    waterMaterial.specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 32.0f);

    boxMaterial.ambient = Convert::ToXmFloat4(Colors::Silver);
    boxMaterial.diffuse = XMFLOAT4(0.2f, 0.4f, 0.6f, 1.0f);
    boxMaterial.specular = XMFLOAT4(0.9f, 0.9f, 0.9f, 16.0f);

    cylinderMaterial.ambient = Convert::ToXmFloat4(Colors::Silver);
    cylinderMaterial.diffuse = XMFLOAT4(0.7f, 0.85f, 0.7f, 1.0f);
    cylinderMaterial.specular = XMFLOAT4(1.0f, 0.8f, 0.8f, 4.0f);

    sphereMaterial.ambient = Convert::ToXmFloat4(Colors::Silver);
    sphereMaterial.diffuse = XMFLOAT4(0.8f, 0.3f, 0.2f, 1.0f);
    sphereMaterial.specular = XMFLOAT4(0.9f, 0.9f, 0.9f, 16.0f);

    mDirectionLight.ambient = Convert::ToXmFloat4(Colors::Silver);
    mDirectionLight.diffuse = XMFLOAT4(0.1f, 0.1f, 0.1f, 0.1f);
    mDirectionLight.specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.f);
    mDirectionLight.direction = XMFLOAT3(1.0f, 0.0f, 0.0f);

    mPointLight.ambient = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
    mPointLight.diffuse = XMFLOAT4(0.4f, 0.1f, 0.1f, 1.0f);
    mPointLight.specular = XMFLOAT4(20.0f, 1.0f, 0.4f, 1.0f);
    mPointLight.position = XMFLOAT3(0.0f, 1.0f, 0.0f);
    mPointLight.range = 16;
    mPointLight.att = XMFLOAT3(0.0f, 0.0f, 1.0f);

    mSpotLight.ambient = XMFLOAT4(1.8f, 1.1f, 0.1f, 1.0f);
    mSpotLight.diffuse = XMFLOAT4(1.8f, 1.8f, 0.1f, 1.0f);
    mSpotLight.specular = XMFLOAT4(20.0f, 20.0f, 0.0f, 1.0f);
    mSpotLight.position = XMFLOAT3(-16.0f, 2.0f, 0.0f);
    mSpotLight.range = 16;
    mSpotLight.direction = XMFLOAT3(1.0f, 0.0f, 0.0f);
    mSpotLight.spot = 6.0f;
    mSpotLight.att = XMFLOAT3(0.0f, 0.0f, 0.5f);

}

bool BasicTextureApp2::LoadContent()
{
    mCamera.SetPosition(0.0f, 2.0f, -10.0f);

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

    D3D11_RASTERIZER_DESC noCullDesc;
    ZeroMemory(&noCullDesc, sizeof(D3D11_RASTERIZER_DESC));
    noCullDesc.FillMode = D3D11_FILL_SOLID;
    noCullDesc.CullMode = D3D11_CULL_NONE;
    noCullDesc.FrontCounterClockwise = false;
    noCullDesc.DepthClipEnable = true;

    HRERROR(md3dDevice->CreateRasterizerState(&noCullDesc, mNoCullRS.resetAndGetPointerAddress()));

    return true;
}

void BasicTextureApp2::UnloadContent()
{
    mNoCullRS = nullptr;

    pEffect = nullptr;

    pLayout = nullptr;
    pVBuffer = nullptr;
    pIBuffer = nullptr;
    pWaterVBuffer = nullptr;
    pWaterIBuffer = nullptr;
}


bool BasicTextureApp2::buildFx()
{
    Windows::ComPtr<ID3DBlob> shader = nullptr;
    Windows::ComPtr<ID3DBlob> errorBlob;
    std::string error;

    D3DX11CompileFromFileW(L"fx/basictexture2.fx", 0, 0, 0, "fx_5_0", 0, 0, 0, shader.resetAndGetPointerAddress(), errorBlob.resetAndGetPointerAddress(), 0);
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
    pEffect->GetTechniqueByName("TextureDraw")->GetPassByIndex(0)->GetDesc(&passDesc);
    HRESULT hr = (md3dDevice->CreateInputLayout(ied, sizeof(ied)/sizeof(D3D11_INPUT_ELEMENT_DESC), passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, pLayout.resetAndGetPointerAddress()));
    if (FAILED(hr))
    {
        return false;
    }

    pWorldMatrix = pEffect->GetVariableByName("worldMatrix")->AsMatrix();
    pWorldInvTranspose = pEffect->GetVariableByName("worldInvTranspose")->AsMatrix();
    pViewMatrix = pEffect->GetVariableByName("viewMatrix")->AsMatrix();
    pProjMatrix = pEffect->GetVariableByName("projectionMatrix")->AsMatrix();
    pTexTransformMatrix = pEffect->GetVariableByName("texTransformMatrix")->AsMatrix();

    pShaderTexture = pEffect->GetVariableByName("image")->AsShaderResource();


    pMaterial = pEffect->GetVariableByName("gMaterial");

    pDirectionLight = pEffect->GetVariableByName("gDirLight");

    pPointLight = pEffect->GetVariableByName("gPointLight");

    pSpotLight = pEffect->GetVariableByName("gSpotLight");

    pCameraPosition = pEffect->GetVariableByName("cameraPosition");

    fogStart = pEffect->GetVariableByName("fogStart")->AsScalar();
    fogRange = pEffect->GetVariableByName("fogRange")->AsScalar();
    fogColor = pEffect->GetVariableByName("fogColor")->AsVector();

    useLighting = pEffect->GetVariableByName("useLighting")->AsScalar();
    useTexture = pEffect->GetVariableByName("useTexture")->AsScalar();
    drawFog = pEffect->GetVariableByName("drawFog")->AsScalar();


    return true;
}

bool BasicTextureApp2::loadModel()
{
    GeometryGenerator::MeshData box;
    GeometryGenerator::MeshData ground;
    GeometryGenerator::MeshData cylinder;
    GeometryGenerator::MeshData sphere;

    GeometryGenerator geoGen;
    geoGen.CreateBox(2.0f, 2.0f, 2.0f, box);
    geoGen.CreateGrid(150.0f, 150.0f, 50, 50, ground);
    geoGen.CreateCylinder(2.0f, 1.5f, 4.0f, 50, 100, cylinder);
    geoGen.createSphere(1.0f, 100, 100, sphere);


    // Cache the vertex offsets to each object in the concatenated vertex buffer.
    mBoxVertexOffset = 0;
    mGroundVertexOffset = mBoxVertexOffset + box.vertices.size();
    mCylinderVertexOffset = mGroundVertexOffset + ground.vertices.size();
    mSphereVertexOffset = mCylinderVertexOffset + cylinder.vertices.size();

    // Cache the index count of each object.
    mBoxIndexCount = box.indices.size();

    mGroundIndexCount = ground.indices.size();
    mCylinderIndexCount = cylinder.indices.size();
    mSphereIndexCount = sphere.indices.size();

    // Cache the starting index for each object in the concatenated index buffer.
    mBoxIndexOffset = 0;
    mGroundIndexOffset = mBoxIndexOffset + mBoxIndexCount;
    mCylinderIndexOffset = mGroundIndexOffset + mGroundIndexCount;
    mSphereIndexOffset = mCylinderIndexOffset + mCylinderIndexCount;

    UINT totalVertexCount =
        box.vertices.size()  + ground.vertices.size() + cylinder.vertices.size() + sphere.vertices.size();

    UINT totalIndexCount =
        mBoxIndexCount  + mGroundIndexCount + mCylinderIndexCount + mSphereIndexCount;

    std::vector<Vertex> vertices(totalVertexCount);

    UINT k = 0;
    for (size_t i = 0; i < box.vertices.size(); ++i, ++k)
    {
        vertices[k].position = box.vertices[i].position;
        vertices[k].normal = box.vertices[i].normal;
        vertices[k].tex = box.vertices[i].tex;
    }


    for (size_t i = 0; i < ground.vertices.size(); ++i, ++k)
    {
        vertices[k].position = ground.vertices[i].position;
        vertices[k].position.y = GetHillHeight(vertices[k].position.x, vertices[k].position.z);
        vertices[k].normal = GetHillNormal(vertices[k].position.x, vertices[k].position.z);
        vertices[k].tex = ground.vertices[i].tex;
    }

    for (size_t i = 0; i < cylinder.vertices.size(); ++i, ++k)
    {
        vertices[k].position = cylinder.vertices[i].position;
        vertices[k].normal = cylinder.vertices[i].normal;
        vertices[k].tex = cylinder.vertices[i].tex;
    }

    for (size_t i = 0; i < sphere.vertices.size(); ++i, ++k)
    {
        vertices[k].position = sphere.vertices[i].position;
        vertices[k].normal = sphere.vertices[i].normal;
        vertices[k].tex = sphere.vertices[i].tex;
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
    indices.insert(indices.end(), ground.indices.begin(), ground.indices.end());
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


    m_water.Init(150, 150, 1.0f, 0.03f, 5.0f, 0.3f);
    std::vector<Vertex> watervertices(m_water.VertexCount());

    for (size_t  i = 0; i < watervertices.size(); i++)
    {
        watervertices[i].position = m_water[i];
        watervertices[i].normal = m_water.Normal(i);
        watervertices[i].tex = m_water.Tex(i);
    }

    std::vector<UINT> waterindices;
    waterindices = m_water.indices();

    D3D11_BUFFER_DESC watervbd;
    watervbd.Usage = D3D11_USAGE_DYNAMIC;
    watervbd.ByteWidth = sizeof(Vertex) * m_water.VertexCount();
    watervbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    watervbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    watervbd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA watervinitData;
    watervinitData.pSysMem = &watervertices[0];
    HRERROR(md3dDevice->CreateBuffer(&watervbd, &watervinitData, pWaterVBuffer.resetAndGetPointerAddress()));

    D3D11_BUFFER_DESC wateribd;
    wateribd.Usage = D3D11_USAGE_IMMUTABLE;
    wateribd.ByteWidth = sizeof(UINT) * m_water.indices().size();
    wateribd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    wateribd.CPUAccessFlags = 0;
    wateribd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA wateriinitData;
    wateriinitData.pSysMem = &waterindices[0];
    HRERROR(md3dDevice->CreateBuffer(&wateribd, &wateriinitData, pWaterIBuffer.resetAndGetPointerAddress()));

    waterTexTransformMaxtrix = XMMatrixIdentity();

    return true;
}


bool BasicTextureApp2::loadResource()
{
    D3DX11_IMAGE_LOAD_INFO loadInfo;
    ZeroMemory(&loadInfo, sizeof(D3DX11_IMAGE_LOAD_INFO));
    loadInfo.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    loadInfo.Format = DXGI_FORMAT_BC2_UNORM;
    loadInfo.MipLevels = D3DX11_DEFAULT;
    loadInfo.MipFilter = D3DX11_FILTER_LINEAR;

    HRERROR(D3DX11CreateShaderResourceViewFromFile(md3dDevice, L"res/water1.dds", &loadInfo, NULL, m_watertexture.resetAndGetPointerAddress(), NULL));
    HRERROR(D3DX11CreateShaderResourceViewFromFile(md3dDevice, L"res/WireFence.dds", &loadInfo, NULL, m_boxtexture.resetAndGetPointerAddress(), NULL));
    HRERROR(D3DX11CreateShaderResourceViewFromFile(md3dDevice, L"res/puppy.jpg", &loadInfo, NULL, m_puppytexture.resetAndGetPointerAddress(), NULL));
    HRERROR(D3DX11CreateShaderResourceViewFromFile(md3dDevice, L"res/grass.dds", &loadInfo, NULL, m_grasstexture.resetAndGetPointerAddress(), NULL));
    HRERROR(D3DX11CreateShaderResourceViewFromFile(md3dDevice, L"res/sphere.png", &loadInfo, NULL, m_spheretexture.resetAndGetPointerAddress(), NULL));

    return true;
}

void BasicTextureApp2::UpdateScene(float timeDelta)
{
    worldMatrix = XMMatrixIdentity();
    viewMatrix = XMMatrixIdentity();
    projectionMatrix = XMMatrixIdentity();

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

    mDirectionLight.direction = XMFLOAT3(cosf(theta), 0.0f, sinf(theta));

    static float px = 0;
    static int dir = 0;
    if (px >= XM_2PI)
    {
        dir = -1;
        px = XM_2PI;
    }
    else if (px <= -XM_2PI)
    {
        dir = 0;
        px = -XM_2PI;
    }
    if (dir == 0)
    {
        px += timeDelta;
    }
    else
    {
        px -= timeDelta;
    }

    mPointLight.position = XMFLOAT3(px * 40.0f / XM_2PI  /*sinf(theta) * 2*/, 10 + 5 * cosf(theta), cosf(theta) * 40.0f);


    mSpotLight.direction = XMFLOAT3(cosf(theta) *cosf(theta), -0.08f, sinf(theta));
    XMStoreFloat3(&mSpotLight.direction, XMVector3Normalize(XMLoadFloat3(&mSpotLight.direction)));

    mCamera.UpdateViewMatrix();

    viewMatrix = mCamera.View();

    float  fieldOfView = (float)XM_PI / ((GetAsyncKeyState('V') & 0x8000) ? 2.0f : 4.0f);
    float screenAspect = (float)mClientWidth / (float)mClientHeight;
    mCamera.SetLens(fieldOfView, screenAspect, 1.0f, 1000.0f);

    projectionMatrix = mCamera.Proj();

    setShaderParams();


    static float t_base = 0.0f;
    if ((mTimer.GameTime() - t_base) >= 0.1f)
    {
        t_base = mTimer.GameTime();

        DWORD i = 5 + rand() % (m_water.RowCount() - 10);
        DWORD j = 5 + rand() % (m_water.ColumnCount() - 10);


        m_water.Disturb(i, j, MathHelper::randfloat(0.5f, 2.0f));
    }

    m_water.Update(timeDelta);

    D3D11_MAPPED_SUBRESOURCE ms;
    if (SUCCEEDED(md3dContext->Map(pWaterVBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms)))
    {
        Vertex* waterVertrices = (Vertex*)ms.pData;

        for (UINT i = 0; i < m_water.VertexCount(); ++i)
        {
            waterVertrices[i].position = m_water[i];
            waterVertrices[i].normal = m_water.Normal(i);
            waterVertrices[i].tex = m_water.Tex(i);
        }

        md3dContext->Unmap(pWaterVBuffer, 0);
    }

    static float waterTexTranlateX = 0.0f;
    static float waterTexTranlateY = 0.0f;

    waterTexTranlateX += timeDelta;
    waterTexTranlateY += timeDelta;

    waterTexTransformMaxtrix = XMMatrixTranslation(waterTexTranlateX, waterTexTranlateY, 0.0);
}

void BasicTextureApp2::setShaderParams()
{
    pViewMatrix->SetMatrix(reinterpret_cast<float*>(&viewMatrix));
    pProjMatrix->SetMatrix(reinterpret_cast<float*>(&projectionMatrix));

    pDirectionLight->SetRawValue(&mDirectionLight, 0, sizeof(mDirectionLight));
    pPointLight->SetRawValue(&mPointLight, 0, sizeof(mPointLight));
    pSpotLight->SetRawValue(&mSpotLight, 0, sizeof(mSpotLight));

    XMFLOAT3 camPos = mCamera.GetPosition();
    pCameraPosition->SetRawValue(&camPos, 0, sizeof(camPos));

    fogStart->SetFloat(15.0f);
    fogRange->SetFloat(150.f);
    fogColor->SetFloatVector((float*)&Colors::Silver);

    useTexture->SetBool(m_useTexture);
    useLighting->SetBool(m_useLighting);
    drawFog->SetBool(m_drawFog);
}

void BasicTextureApp2::DrawScene()
{
    md3dContext->ClearRenderTargetView(mRenderTargetView, D3DXCOLOR(Convert::ToXmColor(Colors::LightSteelBlue)));
    md3dContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

    enableAlpha();

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    md3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    md3dContext->IASetInputLayout(pLayout);


    //std::string techName = GetAsyncKeyState('P')&0x8000 ? "LightingDraw":  m_useLighting ? "TextureDrawWithLight" : "TextureDraw";
    std::string techName = "Draw";

    ID3DX11EffectTechnique *tech = pEffect->GetTechniqueByName(techName.c_str());
    D3DX11_TECHNIQUE_DESC techDesc;
    tech->GetDesc(&techDesc);

    XMMATRIX indentityMatrix = XMMatrixIdentity();

    for (UINT i = 0; i < techDesc.Passes; ++i)
    {
        md3dContext->IASetVertexBuffers(0, 1, pVBuffer.getPointerAdress(), &stride, &offset);
        md3dContext->IASetIndexBuffer(pIBuffer, DXGI_FORMAT_R32_UINT, 0);
       

        //1. draw objects with RS not cull back
        md3dContext->RSSetState(mNoCullRS);

        pTexTransformMatrix->SetMatrix((float*)&indentityMatrix);

       
        {
            XMMATRIX boxWorldMaxtrix = XMMatrixTranslation(0.0f, 1.0f, 0.0f);
            boxWorldMaxtrix = XMMatrixScaling(3.0f, 3.0f, 3.0f) *boxWorldMaxtrix;
            pWorldMatrix->SetMatrix(reinterpret_cast<float*>(&boxWorldMaxtrix));

            XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(boxWorldMaxtrix);
            pWorldInvTranspose->SetMatrix((const float*)&worldInvTranspose);

            pMaterial->SetRawValue(&boxMaterial, 0, sizeof(boxMaterial));
            pShaderTexture->SetResource(m_boxtexture);

            tech->GetPassByIndex(i)->Apply(0, md3dContext);
            md3dContext->DrawIndexed(mBoxIndexCount, mBoxIndexOffset, mBoxVertexOffset);

        }

        {         
            XMMATRIX cylinderWorldMaxtrix = XMMatrixTranslation(15.0f, 2.0f, 0.0f);
            cylinderWorldMaxtrix = XMMatrixScaling(3.0f, 3.0f, 3.0f) *cylinderWorldMaxtrix;
            pWorldMatrix->SetMatrix(reinterpret_cast<float*>(&cylinderWorldMaxtrix));

            XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(cylinderWorldMaxtrix);
            pWorldInvTranspose->SetMatrix((const float*)&worldInvTranspose);

            pMaterial->SetRawValue(&cylinderMaterial, 0, sizeof(cylinderMaterial));
            pShaderTexture->SetResource(m_puppytexture);

            tech->GetPassByIndex(i)->Apply(0, md3dContext);
            md3dContext->DrawIndexed(mCylinderIndexCount, mCylinderIndexOffset, mCylinderVertexOffset);
        }


        {
            XMMATRIX sphereWorldMaxtrix = XMMatrixTranslation(-16.0f, 2.0f, 0.0f);
            sphereWorldMaxtrix = XMMatrixScaling(3.0f, 3.0f, 3.0f) *sphereWorldMaxtrix;
            pWorldMatrix->SetMatrix(reinterpret_cast<float*>(&sphereWorldMaxtrix));

            XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(sphereWorldMaxtrix);
            pWorldInvTranspose->SetMatrix((const float*)&worldInvTranspose);

            pMaterial->SetRawValue(&sphereMaterial, 0, sizeof(sphereMaterial));
            pShaderTexture->SetResource(m_spheretexture);

            tech->GetPassByIndex(i)->Apply(0, md3dContext);
            md3dContext->DrawIndexed(mSphereIndexCount, mSphereIndexOffset, mSphereVertexOffset);
        }

        md3dContext->RSSetState(nullptr);


        //2. draw ground and water
        {
            pWorldMatrix->SetMatrix(reinterpret_cast<float*>(&indentityMatrix));
            
            XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(indentityMatrix);
            pWorldInvTranspose->SetMatrix((const float*)&worldInvTranspose);

            pMaterial->SetRawValue(&groundMaterial, 0, sizeof(groundMaterial));
            pShaderTexture->SetResource(m_grasstexture);

            tech->GetPassByIndex(i)->Apply(0, md3dContext);
            md3dContext->DrawIndexed(mGroundIndexCount, mGroundIndexOffset, mGroundVertexOffset);
        }


        {
            md3dContext->IASetVertexBuffers(0, 1, pWaterVBuffer.getPointerAdress(), &stride, &offset);
            md3dContext->IASetIndexBuffer(pWaterIBuffer, DXGI_FORMAT_R32_UINT, 0);

            pWorldMatrix->SetMatrix(reinterpret_cast<float*>(&indentityMatrix));

            XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(indentityMatrix);
            pWorldInvTranspose->SetMatrix((const float*)&worldInvTranspose);

            pTexTransformMatrix->SetMatrix((float*)&waterTexTransformMaxtrix);

            pMaterial->SetRawValue(&waterMaterial, 0, sizeof(waterMaterial));
            pShaderTexture->SetResource(m_watertexture);
        
            tech->GetPassByIndex(i)->Apply(0, md3dContext);
            md3dContext->DrawIndexed(m_water.indices().size(), 0, 0);
        }
    }

    restoreAlpha();
}

LRESULT BasicTextureApp2::WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_KEYDOWN)
    {
        if (wParam == 'L')
        {
            m_useLighting = !m_useLighting;
        }
        else if (wParam == 'F')
        {
            m_drawFog = !m_drawFog;
        }
        else if (wParam == 'T')
        {
            m_useTexture = !m_useTexture;
        }
    }

    return __super::WindowProc(hwnd, message, wParam, lParam);
}
