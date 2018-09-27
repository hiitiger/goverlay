#pragma once
#include "dxsdk/Include/d3dx9math.h"
#include "sprites.shader.h"

class D3d10SpriteDrawer
{
public:
    D3d10SpriteDrawer(Windows::ComPtr<ID3D10Device> device);
    ~D3d10SpriteDrawer();

    void init(HMODULE hModuleD3dCompiler47, int swapChainWidth, int swapChainHeight);

    void drawUnscaleSprite(Windows::ComPtr<ID3D10Texture2D> texture, RECT& rc, D3DCOLOR color = 0xffffffff);

    void drawScaleSprite(Windows::ComPtr<ID3D10Texture2D> texture, RECT& srcRect, RECT& dstRect, D3DCOLOR color = 0xffffffff);


private:
    Windows::ComPtr<ID3D10Device> m_d3dDevice;
    float m_targetWidth;
    float m_targetHeight;

    Windows::ComPtr<ID3D10InputLayout> m_inputLayout;
    Windows::ComPtr<ID3D10VertexShader> m_VS;
    Windows::ComPtr<ID3D10PixelShader> m_PS;
};



inline D3d10SpriteDrawer::D3d10SpriteDrawer(Windows::ComPtr<ID3D10Device> device)
    : m_d3dDevice(device)
    , m_targetWidth(0)
    , m_targetHeight(0)
{

}

inline D3d10SpriteDrawer::~D3d10SpriteDrawer()
{

}

inline void D3d10SpriteDrawer::init(HMODULE hModuleD3dCompiler47, int swapChainWidth, int swapChainHeight)
{
    m_targetWidth = (float)swapChainWidth;
    m_targetHeight = (float)swapChainHeight;

    Windows::ComPtr<ID3DBlob> VS;
    Windows::ComPtr<ID3DBlob> PS;
    Windows::ComPtr<ID3DBlob> errorBlob;
    std::string error;

    pD3DCompile fn = (pD3DCompile)GetProcAddress(hModuleD3dCompiler47, "D3DCompile");

    if (fn)
    {
        fn(shaderCodeD10.c_str(), shaderCodeD10.size(), nullptr, nullptr, nullptr, "VShader", "vs_4_0", 0, 0, VS.resetAndGetPointerAddress(), errorBlob.resetAndGetPointerAddress());
        if (errorBlob) {
            error = std::string((char*)errorBlob->GetBufferPointer());
            return;
        }
        fn(shaderCodeD10.c_str(), shaderCodeD10.size(), nullptr, nullptr, nullptr, "PShader", "ps_4_0", 0, 0, PS.resetAndGetPointerAddress(), errorBlob.resetAndGetPointerAddress());
        if (errorBlob) {
            error = std::string((char*)errorBlob->GetBufferPointer());
            return;
        }

        // encapsulate both shaders into shader objects
        m_d3dDevice->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), m_VS.resetAndGetPointerAddress());
        m_d3dDevice->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), m_PS.resetAndGetPointerAddress());

        D3D10_INPUT_ELEMENT_DESC ied[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D10_INPUT_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 20, D3D10_INPUT_PER_VERTEX_DATA, 0 },
        };

        m_d3dDevice->CreateInputLayout(ied, 3, VS->GetBufferPointer(), VS->GetBufferSize(), m_inputLayout.resetAndGetPointerAddress());
    }
}

inline void D3d10SpriteDrawer::drawUnscaleSprite(Windows::ComPtr<ID3D10Texture2D> texture, RECT& rc, D3DCOLOR color)
{
    if (!m_VS || !m_PS)
    {
        return;
    }
    D3D10_SHADER_RESOURCE_VIEW_DESC srvDesc;
    memset(&srvDesc, 0, sizeof(srvDesc));
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Texture2D.MostDetailedMip = 0;
    Windows::ComPtr<ID3D10ShaderResourceView> srv;
    m_d3dDevice->CreateShaderResourceView(texture, &srvDesc, srv.resetAndGetPointerAddress());


    Windows::ComPtr<ID3D10Buffer> pVBuffer = nullptr;
    Windows::ComPtr<ID3D10Buffer> pIBuffer = nullptr;

    SpriteVertex vertrices[4];
    vertrices[0].position = PointToNdc(rc.left, rc.top, 0, m_targetWidth, m_targetHeight);
    vertrices[1].position = PointToNdc(rc.right, rc.top, 0, m_targetWidth, m_targetHeight);
    vertrices[2].position = PointToNdc(rc.left, rc.bottom, 0, m_targetWidth, m_targetHeight);
    vertrices[3].position = PointToNdc(rc.right, rc.bottom, 0, m_targetWidth, m_targetHeight);


    D3D10_TEXTURE2D_DESC desc;
    memset(&desc, 0, sizeof(desc));
    texture->GetDesc(&desc);
    float max_u = (float)(rc.right - rc.left) / desc.Width;
    float max_v = (float)(rc.bottom - rc.top) / desc.Height;

    vertrices[0].tex = D3DXVECTOR2(0.0f, 0.0f);
    vertrices[1].tex = D3DXVECTOR2(max_u, 0.0f);
    vertrices[2].tex = D3DXVECTOR2(0.0f, max_v);
    vertrices[3].tex = D3DXVECTOR2(max_u, max_v);

    vertrices[0].color = D3DXCOLOR(color);
    vertrices[1].color = D3DXCOLOR(color);
    vertrices[2].color = D3DXCOLOR(color);
    vertrices[3].color = D3DXCOLOR(color);

    D3D10_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D10_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(SpriteVertex) * 4;
    bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;

    D3D10_SUBRESOURCE_DATA vertexData;
    vertexData.pSysMem = vertrices;
    vertexData.SysMemPitch = 0;
    vertexData.SysMemSlicePitch = 0;

    if (SUCCEEDED(m_d3dDevice->CreateBuffer(&bd, &vertexData, pVBuffer.resetAndGetPointerAddress())))
    {

        m_d3dDevice->IASetInputLayout(m_inputLayout);

        UINT stride = sizeof(SpriteVertex);
        UINT offset = 0;
        m_d3dDevice->IASetVertexBuffers(0, 1, pVBuffer.getPointerAdress(), &stride, &offset);
        m_d3dDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

        m_d3dDevice->VSSetShader(m_VS);
        m_d3dDevice->PSSetShader(m_PS);
        m_d3dDevice->PSSetShaderResources(0, 1, srv.getPointerAdress());

        m_d3dDevice->Draw(4, 0);

    }
}

inline void D3d10SpriteDrawer::drawScaleSprite(Windows::ComPtr<ID3D10Texture2D> texture, RECT& srcRect, RECT& dstRect, D3DCOLOR color /*= 0xffffffff*/)
{
    if (!m_VS || !m_PS)
    {
        return;
    }

    D3D10_SHADER_RESOURCE_VIEW_DESC srvDesc;
    memset(&srvDesc, 0, sizeof(srvDesc));
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Texture2D.MostDetailedMip = 0;
    Windows::ComPtr<ID3D10ShaderResourceView> srv;
    m_d3dDevice->CreateShaderResourceView(texture, &srvDesc, srv.resetAndGetPointerAddress());

    Windows::ComPtr<ID3D10Buffer> pVBuffer = nullptr;
    Windows::ComPtr<ID3D10Buffer> pIBuffer = nullptr;

    SpriteVertex vertrices[4];
    vertrices[0].position = PointToNdc(dstRect.left, dstRect.top, 0, m_targetWidth, m_targetHeight);
    vertrices[1].position = PointToNdc(dstRect.right, dstRect.top, 0, m_targetWidth, m_targetHeight);
    vertrices[2].position = PointToNdc(dstRect.left, dstRect.bottom, 0, m_targetWidth, m_targetHeight);
    vertrices[3].position = PointToNdc(dstRect.right, dstRect.bottom, 0, m_targetWidth, m_targetHeight);


    D3D10_TEXTURE2D_DESC desc;
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

    D3D10_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D10_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(SpriteVertex) * 4;
    bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;

    D3D10_SUBRESOURCE_DATA vertexData;
    vertexData.pSysMem = vertrices;
    vertexData.SysMemPitch = 0;
    vertexData.SysMemSlicePitch = 0;

    if (SUCCEEDED(m_d3dDevice->CreateBuffer(&bd, &vertexData, pVBuffer.resetAndGetPointerAddress())))
    {

        m_d3dDevice->IASetInputLayout(m_inputLayout);

        UINT stride = sizeof(SpriteVertex);
        UINT offset = 0;
        m_d3dDevice->IASetVertexBuffers(0, 1, pVBuffer.getPointerAdress(), &stride, &offset);
        m_d3dDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

        m_d3dDevice->VSSetShader(m_VS);
        m_d3dDevice->PSSetShader(m_PS);
        m_d3dDevice->PSSetShaderResources(0, 1, srv.getPointerAdress());

        m_d3dDevice->Draw(4, 0);
    }
}
