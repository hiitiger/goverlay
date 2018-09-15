#pragma once

class BasicDrawApp : public DxAppBase
{
public:
    BasicDrawApp();

    virtual bool LoadContent();
    virtual void UnloadContent();

    virtual void UpdateScene(float dt);
    virtual void DrawScene();

    bool buildFx();
    bool loadModel();

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

    ID3DX11EffectMatrixVariable	*pWorldViewProj;
    XMMATRIX viewMatrix, projectionMatrix, worldMatrix;

};