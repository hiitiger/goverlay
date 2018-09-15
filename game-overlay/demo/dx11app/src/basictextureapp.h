#pragma once

#include "lighthelper.h"

class BasicTextureApp : public DxAppBase
{
public:

    BasicTextureApp();

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
   
    Windows::ComPtr<ID3D11Buffer> pVBuffer;
    Windows::ComPtr<ID3D11Buffer> pIBuffer;

    int mBoxVertexOffset;
    int mGridVertexOffset;
    int mCylinderVertexOffset;
    int mSphereVertexOffset;

    int mBoxIndexCount;
    int mGridIndexCount;
    int mCylinderIndexCount;
    int mSphereIndexCount;

    int mBoxIndexOffset;
    int mGridIndexOffset;
    int mCylinderIndexOffset;
    int mSphereIndexOffset;


    Windows::ComPtr<ID3D11ShaderResourceView> m_boxtexture;
    Windows::ComPtr<ID3D11ShaderResourceView> m_puppytexture;
    Windows::ComPtr<ID3D11ShaderResourceView> m_skytexture;
    Windows::ComPtr<ID3D11ShaderResourceView> m_spheretexture;

    Windows::ComPtr<ID3DX11Effect> pEffect;

    ID3DX11EffectMatrixVariable	*pWorldMatrix;
    ID3DX11EffectMatrixVariable	*pWorldInvTranspose;
    ID3DX11EffectMatrixVariable	*pViewMatrix;
    ID3DX11EffectMatrixVariable	*pProjMatrix;

    ID3DX11EffectShaderResourceVariable* pShaderTexture;

    ID3DX11EffectVariable	*pCameraPosition;

    ID3DX11EffectVariable* pMaterial;
    ID3DX11EffectVariable* pDirectionLight;
    ID3DX11EffectVariable* pPointLight;
    ID3DX11EffectVariable* pSpotLight;

    XMMATRIX viewMatrix, projectionMatrix, worldMatrix;

    Material groundMaterial;
    Material boxMaterial;
    Material cylinderMaterial;
    Material sphereMaterial;

    DirectionalLight mDirectionLight;
    PointLight mPointLight;
    SpotLight mSpotLight;

    bool m_useLighting;
};