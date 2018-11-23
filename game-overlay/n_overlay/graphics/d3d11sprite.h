#pragma once
#include "dxsdk/Include/d3dx9math.h"
#include "sprites.shader.h"

class D3d11SpriteDrawer
{
public:
    D3d11SpriteDrawer(Windows::ComPtr<ID3D11Device> device, Windows::ComPtr<ID3D11DeviceContext> context);
    ~D3d11SpriteDrawer();

    void init(HMODULE hModuleD3dCompiler47, int swapChainWidth, int swapChainHeight, bool gammaFix);

    void drawUnscaleSprite(Windows::ComPtr<ID3D11Texture2D> texture, RECT& rc, D3DCOLOR color = 0xffffffff);

    void drawScaleSprite(Windows::ComPtr<ID3D11Texture2D> texture, RECT& srcRect, RECT& dstRect, D3DCOLOR color = 0xffffffff);


private:
    Windows::ComPtr<ID3D11Device> m_d3dDevice;
    Windows::ComPtr<ID3D11DeviceContext> m_d3dContext;
    float m_targetWidth;
    float m_targetHeight;

    Windows::ComPtr<ID3D11InputLayout> m_inputLayout;
    Windows::ComPtr<ID3D11VertexShader> m_VS;
    Windows::ComPtr<ID3D11PixelShader> m_PS;

};




inline D3d11SpriteDrawer::D3d11SpriteDrawer(Windows::ComPtr<ID3D11Device> device, Windows::ComPtr<ID3D11DeviceContext> context)
    : m_d3dDevice(device)
    , m_d3dContext(context)
    , m_targetWidth(0)
    , m_targetHeight(0)
{

}

inline D3d11SpriteDrawer::~D3d11SpriteDrawer()
{

}

inline void D3d11SpriteDrawer::init(HMODULE hModuleD3dCompiler47, int swapChainWidth, int swapChainHeight, bool gammaFix)
{
    m_targetWidth = (float)swapChainWidth;
    m_targetHeight = (float)swapChainHeight;

    Windows::ComPtr<ID3DBlob> VS;
    Windows::ComPtr<ID3DBlob> PS;
    Windows::ComPtr<ID3DBlob> errorBlob;
    std::string error;
    HRESULT hr;

    pD3DCompile fn = (pD3DCompile)GetProcAddress(hModuleD3dCompiler47, "D3DCompile");

    if (fn)
    {
        hr = fn(shaderCode.c_str(), shaderCode.size(), nullptr, nullptr, nullptr, "VShader", "vs_4_0", 0, 0, VS.resetAndGetPointerAddress(), errorBlob.resetAndGetPointerAddress());
        if (FAILED(hr)) {
            LOGGER("n_overlay") << L"compile VShader hr:" << hr;
            if (errorBlob) {
                error = std::string((char*)errorBlob->GetBufferPointer());
                std::cout << "compile VShader error:" << error << std::endl;
                LOGGER("n_overlay") << L"compile VShader:" << error;
                return;
            }
        }


        hr = fn(shaderCode.c_str(), shaderCode.size(), nullptr, nullptr, nullptr, gammaFix ? "PShaderG" : "PShader", "ps_4_0", 0, 0, PS.resetAndGetPointerAddress(), errorBlob.resetAndGetPointerAddress());
        if (FAILED(hr))
        {
            LOGGER("n_overlay") << L"compile PShader hr:" << hr;
            if (errorBlob) {
                error = std::string((char*)errorBlob->GetBufferPointer());
                std::cout << "compile PShader error:" << error << std::endl;
                LOGGER("n_overlay") << L"compile PShader:" << error;
            }
            return;
        }

        hr = m_d3dDevice->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, m_VS.resetAndGetPointerAddress());
        if (FAILED(hr))
        {
            LOGGER("n_overlay") << L"CreateVertexShader hr:" << hr;
            return;
        }

        hr = m_d3dDevice->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, m_PS.resetAndGetPointerAddress());
        if (FAILED(hr))
        {
            LOGGER("n_overlay") << L"CreatePixelShader hr:" << hr;
            return;
        }

        D3D11_INPUT_ELEMENT_DESC ied[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };

        hr = m_d3dDevice->CreateInputLayout(ied, 3, VS->GetBufferPointer(), VS->GetBufferSize(), m_inputLayout.resetAndGetPointerAddress());
        if (FAILED(hr))
        {
            LOGGER("n_overlay") << L"CreateInputLayout hr:" << hr;
            return;
        }
    }
}

inline void D3d11SpriteDrawer::drawUnscaleSprite(Windows::ComPtr<ID3D11Texture2D> texture, RECT& rc, D3DCOLOR color)
{
    if (!m_VS || !m_PS)
    {
        return;
    }

    HRESULT hr;
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    memset(&srvDesc, 0, sizeof(srvDesc));
    srvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Texture2D.MostDetailedMip = 0;
    Windows::ComPtr<ID3D11ShaderResourceView> srv;
    hr = m_d3dDevice->CreateShaderResourceView(texture, &srvDesc, srv.resetAndGetPointerAddress());
    if (FAILED(hr))
    {
        LOGGER("n_overlay") << L"drawUnscaleSprite CreateShaderResourceView failed:" << hr;
        std::cout << "drawUnscaleSprite CreateShaderResourceView failed :" << hr << std::endl;
        if (hr == DXGI_ERROR_DEVICE_REMOVED)
        {
            hr = m_d3dDevice->GetDeviceRemovedReason();
            std::cout << "GetDeviceRemovedReason :" << hr << std::endl;
            LOGGER("n_overlay") << L"drawUnscaleSprite CreateShaderResourceView failed GetDeviceRemovedReason:" << hr;
        }
        return;
    }

    Windows::ComPtr<ID3D11Buffer> pVBuffer = nullptr;
    Windows::ComPtr<ID3D11Buffer> pIBuffer = nullptr;

    SpriteVertex vertrices[4];
    vertrices[0].position = PointToNdc(rc.left, rc.top, 0, m_targetWidth, m_targetHeight);
    vertrices[1].position = PointToNdc(rc.right, rc.top, 0, m_targetWidth, m_targetHeight);
    vertrices[2].position = PointToNdc(rc.left, rc.bottom, 0, m_targetWidth, m_targetHeight);
    vertrices[3].position = PointToNdc(rc.right, rc.bottom, 0, m_targetWidth, m_targetHeight);


    D3D11_TEXTURE2D_DESC desc;
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

    hr = m_d3dDevice->CreateBuffer(&bd, &vertexData, pVBuffer.resetAndGetPointerAddress());
    if (SUCCEEDED(hr))
    {
        m_d3dContext->IASetInputLayout(m_inputLayout);

        UINT stride = sizeof(SpriteVertex);
        UINT offset = 0;
        m_d3dContext->IASetVertexBuffers(0, 1, pVBuffer.getPointerAdress(), &stride, &offset);
        m_d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

        m_d3dContext->VSSetShader(m_VS, 0, 0);
        m_d3dContext->PSSetShader(m_PS, 0, 0);
        m_d3dContext->PSSetShaderResources(0, 1, srv.getPointerAdress());

        m_d3dContext->Draw(4, 0);
    }
    else
    {
        std::cout << "drawUnscaleSprite CreateBuffer failed :" << hr << std::endl;
        LOGGER("n_overlay") << "drawUnscaleSprite CreateBuffer failed :" << hr;
        if (hr == DXGI_ERROR_DEVICE_REMOVED)
        {
            hr = m_d3dDevice->GetDeviceRemovedReason();
            std::cout << "GetDeviceRemovedReason :" << hr << std::endl;
        }
    }
}

inline void D3d11SpriteDrawer::drawScaleSprite(Windows::ComPtr<ID3D11Texture2D> texture, RECT& srcRect, RECT& dstRect, D3DCOLOR color /*= 0xffffffff*/)
{
    if (!m_VS || !m_PS)
    {
        return;
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    memset(&srvDesc, 0, sizeof(srvDesc));
    srvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Texture2D.MostDetailedMip = 0;
    Windows::ComPtr<ID3D11ShaderResourceView> srv;
    HRESULT hr = m_d3dDevice->CreateShaderResourceView(texture, &srvDesc, srv.resetAndGetPointerAddress());
    if (FAILED(hr))
    {
        std::cout << "drawScaleSprite CreateShaderResourceView failed :" << hr << std::endl;
        LOGGER("n_overlay") << "drawScaleSprite CreateShaderResourceView failed :" << hr;

        if (hr == DXGI_ERROR_DEVICE_REMOVED)
        {
            hr = m_d3dDevice->GetDeviceRemovedReason();
            std::cout << "GetDeviceRemovedReason :" << hr << std::endl;
            LOGGER("n_overlay") << "drawScaleSprite GetDeviceRemovedReason :" << hr;
        }
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

    hr = (m_d3dDevice->CreateBuffer(&bd, &vertexData, pVBuffer.resetAndGetPointerAddress()));
    if (SUCCEEDED(hr))
    {

        m_d3dContext->IASetInputLayout(m_inputLayout);

        UINT stride = sizeof(SpriteVertex);
        UINT offset = 0;
        m_d3dContext->IASetVertexBuffers(0, 1, pVBuffer.getPointerAdress(), &stride, &offset);
        m_d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

        m_d3dContext->VSSetShader(m_VS, 0, 0);
        m_d3dContext->PSSetShader(m_PS, 0, 0);
        m_d3dContext->PSSetShaderResources(0, 1, srv.getPointerAdress());

        m_d3dContext->Draw(4, 0);
    }
    else
    {
        LOGGER("n_overlay") << "drawScaleSprite CreateBuffer failed:" << hr;
        std::cout << "drawScaleSprite CreateBuffer failed :" << hr << std::endl;
        if (hr == DXGI_ERROR_DEVICE_REMOVED)
        {
            hr = m_d3dDevice->GetDeviceRemovedReason();
            std::cout << "GetDeviceRemovedReason :" << hr << std::endl;
            LOGGER("n_overlay") << "drawScaleSprite GetDeviceRemovedReason:" << hr;
        }
    }
}
