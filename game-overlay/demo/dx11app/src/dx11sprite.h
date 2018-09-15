#pragma once

class DX11Sprite
{
public:
    DX11Sprite(Windows::ComPtr<ID3D11Device> device, Windows::ComPtr<ID3D11DeviceContext> context);
    ~DX11Sprite();

    bool init();

    void begin();

    void end();

    void addTexture(ID3D11Texture2D* texture, ID3D11ShaderResourceView* srv);
    void removeTexture(ID3D11Texture2D* texture);

    void drawUnscaleSprite(ID3D11Texture2D* texture, RECT& dst, XMMATRIX* matrix = nullptr, D3DXCOLOR color = 0xffffffff);

    void drawScaleSprite(ID3D11Texture2D* texture, RECT& srcRect, RECT& dstRect, XMMATRIX* matrix = nullptr, D3DXCOLOR color = 0xffffffff);

  
private:
    Windows::ComPtr<ID3D11Device> m_d3dDevice;
    Windows::ComPtr<ID3D11DeviceContext> m_d3dContext;
    float m_targetWidth;
    float m_targetHeight;

    Windows::ComPtr<ID3D11InputLayout> m_inputLayout;
    Windows::ComPtr<ID3D11VertexShader> m_VS;
    Windows::ComPtr<ID3D11PixelShader> m_PS;

    Windows::ComPtr<ID3DX11Effect> m_effect;
    ID3DX11EffectShaderResourceVariable* m_shaderTexture;
    ID3DX11EffectMatrixVariable* m_shaderMaxtrix;

    struct TextureMapElement
    {
        Windows::ComPtr<ID3D11ShaderResourceView> srv;
    };


    std::map<ID3D11Texture2D*, TextureMapElement> m_textureMap;
};