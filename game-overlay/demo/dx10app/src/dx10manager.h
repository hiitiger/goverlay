/************************************************************************************
*	DirectX 10 Manager Class
*	----------------------------------------
*	code by : bobby anguelov - banguelov@cs.up.ac.za
*	downloaded from : takinginitiative.wordpress.org
*
*	code is free for use in whatever way you want, however if you work for a game
*	development firm you are obliged to offer me a job :P (haha i wish)
************************************************************************************/

#ifndef DXMANAGER
#define DXMANAGER

#include <windows.h>
#include <d3d10.h>

#include <vector>

#include "vertexTypes.h"

class dxManager
{
    /*******************************************************************
    * Members
    ********************************************************************/
private:

    //window handle shortcut
    HWND * hWnd;

    //device vars
    ID3D10Device*				pD3DDevice;
    IDXGISwapChain*				pSwapChain;
    ID3D10RenderTargetView*		pRenderTargetView;
    ID3D10Texture2D*			pDepthStencil;
    ID3D10DepthStencilView*		pDepthStencilView;
    D3D10_VIEWPORT				viewPort;
    ID3D10RasterizerState*		pRS;

    //input layout and mesh
    ID3D10InputLayout*			pVertexLayout;

    //effects and techniques
    ID3D10Effect*				pBasicEffect;
    ID3D10EffectTechnique*		pBasicTechnique;

    //effect variable pointers
    ID3D10EffectMatrixVariable* pViewMatrixEffectVariable;
    ID3D10EffectMatrixVariable* pProjectionMatrixEffectVariable;
    ID3D10EffectMatrixVariable* pWorldMatrixEffectVariable;

    //projection and view matrices
    D3DXMATRIX					worldMatrix;
    D3DXMATRIX                  viewMatrix;
    D3DXMATRIX                  projectionMatrix;

    //technique
    D3D10_TECHNIQUE_DESC		techDesc;

    //scene objects
    ID3DX10Mesh*				pCubeMesh;
    ID3D10ShaderResourceView    *pDiffuseTex, *pAlphaTex;
    ID3D10EffectShaderResourceVariable* pTextureSR;

    /*******************************************************************
    * Methods
    ********************************************************************/
public:

    //constructor and destructor
    dxManager();
    ~dxManager();

    //initialize directx device
    bool initialize(HWND*);

    //scene function
    void renderScene();

private:

    //initialization methods
    bool createSwapChainAndDevice(UINT width, UINT height);
    bool loadShadersAndCreateInputLayouts();
    void createViewports(UINT width, UINT height);
    void initRasterizerState();
    bool createRenderTargetsAndDepthBuffer(UINT width, UINT height);
    bool initializeObjects();

    //fatal error handler
    bool fatalError(const LPCSTR msg);
};

#endif