#include "stable.h"
#include "dx11sprite.h"
#include <D3DCompiler.h>


struct  SpriteVertex
{
    D3DXVECTOR3 position;
    D3DXVECTOR2 tex;
    D3DXCOLOR color;
};

DX11Sprite::DX11Sprite(Windows::ComPtr<ID3D11Device> device, Windows::ComPtr<ID3D11DeviceContext> context)
    : m_d3dDevice(device)
    , m_d3dContext(context)
    , m_targetWidth(0)
    , m_targetHeight(0)
    , m_shaderTexture(nullptr)
{
  
}

DX11Sprite::~DX11Sprite()
{

}

bool DX11Sprite::init( )
{
    Windows::ComPtr<ID3DBlob> shader;
    Windows::ComPtr<ID3DBlob> errorBlob;
    std::string error;

    HRESULT hr = D3DX11CompileFromFileW((L"fx/sprite.fx"), 0, 0, 0, "fx_5_0", D3DCOMPILE_DEBUG, 0, 0, shader.resetAndGetPointerAddress(), errorBlob.resetAndGetPointerAddress(), 0);
    if (FAILED(hr))
    {
        if (errorBlob)
        {
            error = std::string((char*)errorBlob->GetBufferPointer());
        }
        return false;
    }

    HRERROR(D3DX11CreateEffectFromMemory(shader->GetBufferPointer(), shader->GetBufferSize(), 0, m_d3dDevice, m_effect.resetAndGetPointerAddress()));

    D3D11_INPUT_ELEMENT_DESC ied[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    D3DX11_PASS_DESC passDesc = { 0 };

    m_effect->GetTechniqueByName("SpriteTech")->GetPassByIndex(0)->GetDesc(&passDesc);
    m_d3dDevice->CreateInputLayout(ied, 3, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, m_inputLayout.resetAndGetPointerAddress());

    m_shaderTexture = m_effect->GetVariableByName("SpriteTex")->AsShaderResource();
    m_shaderMaxtrix = m_effect->GetVariableByName("transformMatrix")->AsMatrix();

    return true;
}


void DX11Sprite::begin()
{
    Windows::ComPtr<ID3D11RenderTargetView> renderTargetView;
    m_d3dContext->OMGetRenderTargets(
    1,
    renderTargetView.resetAndGetPointerAddress(),
    nullptr
    );

    Windows::ComPtr<ID3D11Resource> renderTarget;
    renderTargetView->GetResource(renderTarget.resetAndGetPointerAddress());

    Windows::ComPtr<ID3D11Texture2D> renderTargetTexture;
    renderTarget->QueryInterface(renderTargetTexture.resetAndGetPointerAddress());

    D3D11_TEXTURE2D_DESC renderTargetTextureDesc;
    renderTargetTexture->GetDesc(&renderTargetTextureDesc);

    m_targetWidth = (float)renderTargetTextureDesc.Width;
    m_targetHeight = (float)renderTargetTextureDesc.Height;
}

void DX11Sprite::end()
{

}

void DX11Sprite::addTexture(ID3D11Texture2D* texture, ID3D11ShaderResourceView* srv)
{
    TextureMapElement elem;
    if (srv)
    {
        elem.srv = srv;
    }
    else
    {
        CD3D11_SHADER_RESOURCE_VIEW_DESC mapElementSrvDesc(
            texture,
            D3D11_SRV_DIMENSION_TEXTURE2D
            );
        m_d3dDevice->CreateShaderResourceView(texture, &mapElementSrvDesc, elem.srv.resetAndGetPointerAddress());
    }

    m_textureMap[texture] = elem;
}

void DX11Sprite::removeTexture(ID3D11Texture2D* texture)
{
    m_textureMap.erase(texture);
}

D3DXVECTOR3 PointToNdc(int x, int y, float z, float targetWidth, float targetHeight)
{
    float X = 2.0f * (float)x / targetWidth - 1.0f;
    float Y = 1.0f - 2.0f * (float)y / targetHeight;
    float Z = z;

    return D3DXVECTOR3(X, Y, Z);
}

void DX11Sprite::drawUnscaleSprite(ID3D11Texture2D* texture, RECT& dst, XMMATRIX* matrix , D3DXCOLOR color /*= 0xffffffff*/)
{
    TextureMapElement element = m_textureMap[texture];
    ID3D11ShaderResourceView* textureView = element.srv;
    if (!textureView)
    {
        return;
    }

    Windows::ComPtr<ID3D11Buffer> pVBuffer = nullptr;
    Windows::ComPtr<ID3D11Buffer> pIBuffer = nullptr;

    SpriteVertex vertrices[4] ;
    vertrices[0].position = PointToNdc(dst.left, dst.top, 0, m_targetWidth, m_targetHeight);
    vertrices[1].position = PointToNdc(dst.right, dst.top, 0, m_targetWidth, m_targetHeight);
    vertrices[2].position = PointToNdc(dst.left, dst.bottom, 0, m_targetWidth, m_targetHeight);
    vertrices[3].position = PointToNdc(dst.right, dst.bottom, 0, m_targetWidth, m_targetHeight);

    D3D11_TEXTURE2D_DESC desc;
    memset(&desc, 0, sizeof(desc));
    texture->GetDesc(&desc);
    float max_u = (float)(dst.right - dst.left) / desc.Width;
    float max_v = (float)(dst.bottom - dst.top) / desc.Height;

    vertrices[0].tex = D3DXVECTOR2(0.0f, 0.0f);
    vertrices[1].tex = D3DXVECTOR2(max_u, 0.0f);
    vertrices[2].tex = D3DXVECTOR2(0.0f, max_v);
    vertrices[3].tex = D3DXVECTOR2(max_u, max_v);

    vertrices[0].color = D3DXCOLOR(color);
    vertrices[1].color = D3DXCOLOR(color);
    vertrices[2].color = D3DXCOLOR(color);
    vertrices[3].color = D3DXCOLOR(color);

    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(SpriteVertex) * 4;  
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0; 

    D3D11_SUBRESOURCE_DATA vertexData;
    vertexData.pSysMem = vertrices;
    vertexData.SysMemPitch = 0;
    vertexData.SysMemSlicePitch = 0;

    if (SUCCEEDED(m_d3dDevice->CreateBuffer(&bd, &vertexData, pVBuffer.resetAndGetPointerAddress())))
    {

        m_d3dContext->IASetInputLayout(m_inputLayout);

        UINT stride = sizeof(SpriteVertex);
        UINT offset = 0;
        m_d3dContext->IASetVertexBuffers(0, 1, pVBuffer.getPointerAdress(), &stride, &offset);
        m_d3dContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

        m_shaderTexture->SetResource(textureView);
        if (matrix)
            m_shaderMaxtrix->SetMatrix((float*)matrix);
        else {
            XMMATRIX indentityMat = XMMatrixIdentity();
            m_shaderMaxtrix->SetMatrix((float*)&indentityMat);
        };
        ID3DX11EffectTechnique *tech = m_effect->GetTechniqueByName("SpriteTech");
        D3DX11_TECHNIQUE_DESC techDesc;
        tech->GetDesc(&techDesc);
        for (UINT i = 0; i < techDesc.Passes; ++i)
        {
            tech->GetPassByIndex(i)->Apply(0, m_d3dContext);
            m_d3dContext->Draw(4, 0);
        }
    }
}


void DX11Sprite::drawScaleSprite(ID3D11Texture2D* texture, RECT& srcRect, RECT& dstRect, XMMATRIX* matrix, D3DXCOLOR color /*= 0xffffffff*/)
{
    TextureMapElement element = m_textureMap[texture];
    ID3D11ShaderResourceView* textureView = element.srv;
    if (!textureView)
    {
        return;
    }

    Windows::ComPtr<ID3D11Buffer> pVBuffer = nullptr;
    Windows::ComPtr<ID3D11Buffer> pIBuffer = nullptr;

    SpriteVertex vertrices[4];
    vertrices[0].position = PointToNdc(dstRect.left, dstRect.top, 0, m_targetWidth, m_targetHeight);
    vertrices[1].position = PointToNdc(dstRect.right, dstRect.top, 0, m_targetWidth, m_targetHeight);
    vertrices[2].position = PointToNdc(dstRect.left, dstRect.bottom, 0, m_targetWidth, m_targetHeight);
    vertrices[3].position = PointToNdc(dstRect.right, dstRect.bottom, 0, m_targetWidth, m_targetHeight);


    D3D11_TEXTURE2D_DESC desc;
    memset(&desc, 0, sizeof(desc));
    texture->GetDesc(&desc);

    float min_u = (float)srcRect.left / desc.Width;
    float min_v = (float)srcRect.top / desc.Height;

    float max_u = (float)(srcRect.right) / desc.Width;
    float max_v = (float)(srcRect.bottom) / desc.Height;

    vertrices[0].tex = D3DXVECTOR2(min_u, min_v);
    vertrices[1].tex = D3DXVECTOR2(max_u, min_v);
    vertrices[2].tex = D3DXVECTOR2(min_u, max_v);
    vertrices[3].tex = D3DXVECTOR2(max_u, max_v);

    vertrices[0].color = D3DXCOLOR(color);
    vertrices[1].color = D3DXCOLOR(color);
    vertrices[2].color = D3DXCOLOR(color);
    vertrices[3].color = D3DXCOLOR(color);

    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(SpriteVertex) * 4;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA vertexData;
    vertexData.pSysMem = vertrices;
    vertexData.SysMemPitch = 0;
    vertexData.SysMemSlicePitch = 0;

    if (SUCCEEDED(m_d3dDevice->CreateBuffer(&bd, &vertexData, pVBuffer.resetAndGetPointerAddress())))
    {

        m_d3dContext->IASetInputLayout(m_inputLayout);

        UINT stride = sizeof(SpriteVertex);
        UINT offset = 0;
        m_d3dContext->IASetVertexBuffers(0, 1, pVBuffer.getPointerAdress(), &stride, &offset);
        m_d3dContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

        m_shaderTexture->SetResource(textureView);
        if(matrix)
            m_shaderMaxtrix->SetMatrix((float*)matrix);
        else {
            XMMATRIX indentityMat = XMMatrixIdentity();
            m_shaderMaxtrix->SetMatrix((float*)&indentityMat);
        }

        ID3DX11EffectTechnique *tech = m_effect->GetTechniqueByName("SpriteTech");
        D3DX11_TECHNIQUE_DESC techDesc;
        tech->GetDesc(&techDesc);
        for (UINT i = 0; i < techDesc.Passes; ++i)
        {
            tech->GetPassByIndex(i)->Apply(0, m_d3dContext);
            m_d3dContext->Draw(4, 0);
        }
    }
}

