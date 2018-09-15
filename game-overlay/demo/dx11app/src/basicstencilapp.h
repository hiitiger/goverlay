#pragma once

#include "lighthelper.h"
#include "common/waves.h"

class BasicStencilApp : public DxAppBase
{
public:
    BasicStencilApp();

    virtual bool LoadContent();
    virtual void UnloadContent();

    bool initState();
    bool buildFx();
    bool loadModel();
    bool loadResource();

    void setShaderParams();

    virtual void UpdateScene(float dt);
    virtual void DrawScene();

    void _drawScene();
    void _drawMirror();

    virtual LRESULT WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
    Windows::ComPtr<ID3D11BlendState> pNoRenderTargetWritesBS;

    Windows::ComPtr<ID3D11DepthStencilState> pMarkMirrorDSS;
    Windows::ComPtr<ID3D11DepthStencilState> pDrawReflectionDSS;
    Windows::ComPtr<ID3D11DepthStencilState> pNoDoubleDDS;

    Windows::ComPtr<ID3D11RasterizerState> pNoCullRS;
    Windows::ComPtr<ID3D11RasterizerState> pCullClockwiseRS;

    Windows::ComPtr<ID3D11InputLayout> pLayout;
  
    Windows::ComPtr<ID3D11Buffer> pVBuffer;

    Windows::ComPtr<ID3D11Buffer> pModelVBuffer;
    Windows::ComPtr<ID3D11Buffer> pModelIBuffer;
    int modelIndexCount;

     
    Windows::ComPtr<ID3D11ShaderResourceView> m_modeltexture;
    Windows::ComPtr<ID3D11ShaderResourceView> m_floortexture;
    Windows::ComPtr<ID3D11ShaderResourceView> m_mirrortexture;
    Windows::ComPtr<ID3D11ShaderResourceView> m_walltexture;

    Windows::ComPtr<ID3DX11Effect> pEffect;

    ID3DX11EffectMatrixVariable	*pWorldMatrix;
    ID3DX11EffectMatrixVariable	*pViewMatrix;
    ID3DX11EffectMatrixVariable	*pProjMatrix;
    ID3DX11EffectMatrixVariable	*pTexTransformMatrix;
    ID3DX11EffectMatrixVariable	*pWorldInvTranspose;
   
    ID3DX11EffectShaderResourceVariable* pShaderTexture;

    ID3DX11EffectVariable	*pCameraPosition;

    ID3DX11EffectVariable* pMaterial;
    ID3DX11EffectVariable* pDirectionLight;
    ID3DX11EffectVariable* pPointLight;
    ID3DX11EffectVariable* pSpotLight;

    XMMATRIX viewMatrix, projectionMatrix, worldMatrix;
    XMMATRIX waterTexTransformMaxtrix;
    XMMATRIX modelWorldMatrix;

    Material modelMaterial;

    Material floorMtrl;
    Material wallMtrl;
    Material mirrorMtrl;
    Material shadowMat;

    DirectionalLight mDirectionLight;

    bool m_useLighting;
};