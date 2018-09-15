#pragma once

#include "lighthelper.h"
#include "common/waves.h"

class BasicTextureApp2 : public DxAppBase
{
public:

    BasicTextureApp2();

    virtual bool LoadContent();
    virtual void UnloadContent();

    bool buildFx();
    bool loadModel();
    bool loadResource();

    void setShaderParams();

    virtual void UpdateScene(float dt);
    virtual void DrawScene();

    virtual LRESULT WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

private:

    Windows::ComPtr<ID3D11InputLayout> pLayout;
    Windows::ComPtr<ID3D11RasterizerState> mNoCullRS;

    Windows::ComPtr<ID3D11Buffer> pVBuffer;
    Windows::ComPtr<ID3D11Buffer> pIBuffer; 
     
    Windows::ComPtr<ID3D11Buffer> pWaterVBuffer;
    Windows::ComPtr<ID3D11Buffer> pWaterIBuffer;

    int mBoxVertexOffset;
    int mGroundVertexOffset;
    int mCylinderVertexOffset;
    int mSphereVertexOffset;

    int mBoxIndexCount;
    int mGroundIndexCount;
    int mCylinderIndexCount;
    int mSphereIndexCount;

    int mBoxIndexOffset;
    int mGroundIndexOffset;
    int mCylinderIndexOffset;
    int mSphereIndexOffset;

    Waves m_water;

    Windows::ComPtr<ID3D11ShaderResourceView> m_watertexture;

    Windows::ComPtr<ID3D11ShaderResourceView> m_boxtexture;
    Windows::ComPtr<ID3D11ShaderResourceView> m_puppytexture;
    Windows::ComPtr<ID3D11ShaderResourceView> m_grasstexture;
    Windows::ComPtr<ID3D11ShaderResourceView> m_spheretexture;

    Windows::ComPtr<ID3DX11Effect> pEffect;

    ID3DX11EffectMatrixVariable	*pWorldMatrix;
    ID3DX11EffectMatrixVariable	*pWorldInvTranspose;
    ID3DX11EffectMatrixVariable	*pViewMatrix;
    ID3DX11EffectMatrixVariable	*pProjMatrix;
    ID3DX11EffectMatrixVariable	*pTexTransformMatrix;

    ID3DX11EffectShaderResourceVariable* pShaderTexture;

    ID3DX11EffectVariable	*pCameraPosition;

    ID3DX11EffectVariable* pMaterial;
    ID3DX11EffectVariable* pDirectionLight;
    ID3DX11EffectVariable* pPointLight;
    ID3DX11EffectVariable* pSpotLight;

    //Îí²ÎÊý
    ID3DX11EffectScalarVariable* fogStart;
    ID3DX11EffectScalarVariable* fogRange;
    ID3DX11EffectVectorVariable* fogColor;

    ID3DX11EffectScalarVariable* useLighting;
    ID3DX11EffectScalarVariable* useTexture;
    ID3DX11EffectScalarVariable* drawFog;

    XMMATRIX viewMatrix, projectionMatrix, worldMatrix, waterTexTransformMaxtrix;

    Material groundMaterial;
    Material waterMaterial;
    Material boxMaterial;
    Material cylinderMaterial;
    Material sphereMaterial;

    DirectionalLight mDirectionLight;
    PointLight mPointLight;
    SpotLight mSpotLight;

    bool m_useLighting = true;
    bool m_useTexture = true;
    bool m_drawFog = true;
};