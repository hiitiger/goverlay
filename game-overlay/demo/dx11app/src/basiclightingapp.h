#pragma once
#include "lighthelper.h"


class BasicLightingApp : public DxAppBase
{
public:
 
    BasicLightingApp();

    virtual bool LoadContent();
    virtual void UnloadContent();

    bool buildFx();
    bool loadModel();
    void setShaderParams();

    virtual void UpdateScene(float dt);
    virtual void DrawScene();

private:

    Windows::ComPtr<ID3D11InputLayout> pLayout;
    Windows::ComPtr<ID3D11RasterizerState> rsState;

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

    Windows::ComPtr<ID3DX11Effect> pEffect;

    ID3DX11EffectMatrixVariable	*pWorldMaxtrix;
    ID3DX11EffectMatrixVariable	*pWorldInvTranspose;
    ID3DX11EffectMatrixVariable	*pViewMaxtrix;
    ID3DX11EffectMatrixVariable	*pProjMaxtrix;

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
};