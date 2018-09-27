#include "dx10Manager.h"
#include <math.h>
using namespace std;

/*******************************************************************
* Constructor
*******************************************************************/
dxManager::dxManager() : pD3DDevice(NULL),
pSwapChain(NULL),
pRenderTargetView(NULL),
pVertexLayout(0),
pBasicEffect(0),
pRS(0),
pDepthStencil(0),
pCubeMesh(0)
{
    D3DXMatrixIdentity(&worldMatrix);
    D3DXMatrixIdentity(&projectionMatrix);
    D3DXMatrixIdentity(&viewMatrix);
}
/*******************************************************************
* Destructor
*******************************************************************/
dxManager::~dxManager()
{
    if (pRenderTargetView) pRenderTargetView->Release();
    if (pSwapChain) pSwapChain->Release();
    if (pD3DDevice) pD3DDevice->Release();
    if (pVertexLayout) pVertexLayout->Release();
    if (pRS) pRS->Release();
    if (pBasicEffect) pBasicEffect->Release();
    if (pDepthStencil) pDepthStencil->Release();

    //free mesh resources
    if (pCubeMesh) pCubeMesh->Release();
}
/*******************************************************************
* Initializes Direct3D Device
*******************************************************************/
bool dxManager::initialize(HWND* hW)
{
    //window handle
    hWnd = hW;

    //get window dimensions
    RECT rc;
    GetClientRect(*hWnd, &rc);
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;

    // CREATE DEVICE
    //*****************************************************************************
    if (!createSwapChainAndDevice(width, height)) return false;

    // INPUT ASSEMBLY STAGE
    //*****************************************************************************
    if (!loadShadersAndCreateInputLayouts()) return false;

    // RASTERIZER STAGE SETUP
    //*****************************************************************************
    createViewports(width, height);
    initRasterizerState();

    // OUTPUT-MERGER STAGE
    //*****************************************************************************
    if (!createRenderTargetsAndDepthBuffer(width, height)) return false;

    // Set up the view and projection matrices
    //*****************************************************************************
    const D3DXVECTOR3 camera[] = { D3DXVECTOR3(0.0f, 0.0f, 0.0f),
        D3DXVECTOR3(0.0f, 0.0f, 1.0f),
        D3DXVECTOR3(0.0f, 1.0f, 0.0f) };

    D3DXMatrixLookAtLH(&viewMatrix, &camera[0], &camera[1], &camera[2]);
    D3DXMatrixPerspectiveFovLH(&projectionMatrix, (float)D3DX_PI * 0.5f, (float)width / (float)height, 0.1f, 20.0f);

    //set shader matrices
    pViewMatrixEffectVariable->SetMatrix(viewMatrix);
    pProjectionMatrixEffectVariable->SetMatrix(projectionMatrix);

    // Initialize Scene Objects
    //*****************************************************************************

    if (!initializeObjects()) return false;

    //everything completed successfully
    return true;
}
/*******************************************************************
* Create Swap Chain and D3D device
*******************************************************************/
bool dxManager::createSwapChainAndDevice(UINT width, UINT height)
{
    //Set up DX swap chain
    //--------------------------------------------------------------
    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

    //set buffer dimensions and format
    swapChainDesc.BufferCount = 2;
    swapChainDesc.BufferDesc.Width = width;
    swapChainDesc.BufferDesc.Height = height;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;;

    //set refresh rate
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;

    //sampling settings
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.SampleDesc.Count = 1;

    //output window handle
    swapChainDesc.OutputWindow = *hWnd;
    swapChainDesc.Windowed = true;

    //Create the D3D device
    //--------------------------------------------------------------
    if (FAILED(D3D10CreateDeviceAndSwapChain(NULL,
        D3D10_DRIVER_TYPE_HARDWARE,
        NULL,
        0,
        D3D10_SDK_VERSION,
        &swapChainDesc,
        &pSwapChain,
        &pD3DDevice))) return fatalError("D3D device creation failed");
    return true;
}
/*******************************************************************
* Set up Viewports
*******************************************************************/
void dxManager::createViewports(UINT width, UINT height)
{
    //create viewport structure	
    viewPort.Width = width;
    viewPort.Height = height;
    viewPort.MinDepth = 0.0f;
    viewPort.MaxDepth = 1.0f;
    viewPort.TopLeftX = 0;
    viewPort.TopLeftY = 0;

    pD3DDevice->RSSetViewports(1, &viewPort);
}
/*******************************************************************
* Initialize Rasterizer State
*******************************************************************/
void dxManager::initRasterizerState()
{
    //set rasterizer	
    D3D10_RASTERIZER_DESC rasterizerState;
    rasterizerState.CullMode = D3D10_CULL_NONE;
    rasterizerState.FillMode = D3D10_FILL_SOLID;
    rasterizerState.FrontCounterClockwise = false;
    rasterizerState.DepthBias = false;
    rasterizerState.DepthBiasClamp = 0;
    rasterizerState.SlopeScaledDepthBias = 0;
    rasterizerState.DepthClipEnable = true;
    rasterizerState.ScissorEnable = false;
    rasterizerState.MultisampleEnable = false;
    rasterizerState.AntialiasedLineEnable = true;

    pD3DDevice->CreateRasterizerState(&rasterizerState, &pRS);
    pD3DDevice->RSSetState(pRS);

    //create blend state
}
/*******************************************************************
* Create Rendering Targets
*******************************************************************/
bool dxManager::createRenderTargetsAndDepthBuffer(UINT width, UINT height)
{
    //try to get the back buffer
    ID3D10Texture2D* pBackBuffer;
    if (FAILED(pSwapChain->GetBuffer(0, __uuidof(ID3D10Texture2D), (LPVOID*)&pBackBuffer))) return fatalError("Could not get back buffer");

    //try to create render target view
    if (FAILED(pD3DDevice->CreateRenderTargetView(pBackBuffer, NULL, &pRenderTargetView))) return fatalError("Could not create render target view");

    pBackBuffer->Release();

    //create depth stencil texture
    D3D10_TEXTURE2D_DESC descDepth;
    descDepth.Width = width;
    descDepth.Height = height;
    descDepth.MipLevels = 1;
    descDepth.ArraySize = 1;
    descDepth.Format = DXGI_FORMAT_D32_FLOAT;
    descDepth.SampleDesc.Count = 1;
    descDepth.SampleDesc.Quality = 0;
    descDepth.Usage = D3D10_USAGE_DEFAULT;
    descDepth.BindFlags = D3D10_BIND_DEPTH_STENCIL;
    descDepth.CPUAccessFlags = 0;
    descDepth.MiscFlags = 0;

    if (FAILED(pD3DDevice->CreateTexture2D(&descDepth, NULL, &pDepthStencil)))  return fatalError("Could not create depth stencil texture");

    // Create the depth stencil view
    D3D10_DEPTH_STENCIL_VIEW_DESC descDSV;
    descDSV.Format = descDepth.Format;
    descDSV.ViewDimension = D3D10_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;

    if (FAILED(pD3DDevice->CreateDepthStencilView(pDepthStencil, &descDSV, &pDepthStencilView))) return fatalError("Could not create depth stencil view");

    //set render targets
    pD3DDevice->OMSetRenderTargets(1, &pRenderTargetView, pDepthStencilView);

    return true;
}
/*******************************************************************
* Shader Loader
*******************************************************************/
bool dxManager::loadShadersAndCreateInputLayouts()
{
    if (FAILED(D3DX10CreateEffectFromFileA("basicEffect.fx",
        NULL, NULL,
        "fx_4_0",
        D3D10_SHADER_ENABLE_STRICTNESS,
        0,
        pD3DDevice,
        NULL,
        NULL,
        &pBasicEffect,
        NULL,
        NULL))) return fatalError("Could not load effect file!");

    pBasicTechnique = pBasicEffect->GetTechniqueByName("render");
    if (pBasicTechnique == NULL) return fatalError("Could not find specified technique!");


    //create matrix effect pointers
    pViewMatrixEffectVariable = pBasicEffect->GetVariableByName("View")->AsMatrix();
    pProjectionMatrixEffectVariable = pBasicEffect->GetVariableByName("Projection")->AsMatrix();
    pWorldMatrixEffectVariable = pBasicEffect->GetVariableByName("World")->AsMatrix();
    pTextureSR = pBasicEffect->GetVariableByName("tex2D")->AsShaderResource();

    //create input layout
    D3D10_PASS_DESC PassDesc;
    pBasicTechnique->GetPassByIndex(0)->GetDesc(&PassDesc);
    if (FAILED(pD3DDevice->CreateInputLayout(vertexInputLayout,
        numInputLayoutElements,
        PassDesc.pIAInputSignature,
        PassDesc.IAInputSignatureSize,
        &pVertexLayout))) return fatalError("Could not create Input Layout!");

    // Set the input layout
    pD3DDevice->IASetInputLayout(pVertexLayout);

    //get technique description
    pBasicTechnique->GetDesc(&techDesc);

    return true;
}
/*******************************************************************
* Initialize Scene Objects
*******************************************************************/
bool dxManager::initializeObjects()
{
    //vertices and indices for two planes
    vertex vb[16] = {	//front verts of building
        vertex(D3DXVECTOR3(-1,-1,0), D3DXVECTOR4(1,0,0,1), D3DXVECTOR2(0.0f, 1.0f)),
        vertex(D3DXVECTOR3(-1,1,0), D3DXVECTOR4(1,0,0,1), D3DXVECTOR2(0.0f, 0.0f)),
        vertex(D3DXVECTOR3(1,1,0), D3DXVECTOR4(1,0,0,1), D3DXVECTOR2(1.0f, 0.0f)),
        vertex(D3DXVECTOR3(1,-1,0), D3DXVECTOR4(1,0,0,1), D3DXVECTOR2(1.0f, 1.0f)),
    };

    UINT ib[42] = { //plane 1
        0,1,3, 3,1,2
    };

    //create cube mesh
    if (FAILED(D3DX10CreateMesh(pD3DDevice, vertexInputLayout, numInputLayoutElements, "POSITION", 4, 2, D3DX10_MESH_32_BIT, &pCubeMesh))) return fatalError("Could not create mesh!");

    //insert data into cube mesh and commit changes
    pCubeMesh->SetVertexData(0, vb);
    pCubeMesh->SetIndexData(ib, 6);
    pCubeMesh->GenerateAdjacencyAndPointReps(0.01f);
    pCubeMesh->Optimize(D3DX10_MESHOPT_ATTR_SORT, NULL, NULL);
    pCubeMesh->CommitToDevice();

    //load textures
    if (FAILED(D3DX10CreateShaderResourceViewFromFileA(pD3DDevice, "textures/diffuse.bmp", NULL, NULL, &pDiffuseTex, NULL))
        || FAILED(D3DX10CreateShaderResourceViewFromFileA(pD3DDevice, "textures/alpha.png", NULL, NULL, &pAlphaTex, NULL))
        )
    {
        return fatalError("Could Not Load Texture");
    }

    return true;
}
/*******************************************************************
* Scene Renderer
*******************************************************************/
void dxManager::renderScene()
{
    //clear scene
    pD3DDevice->ClearRenderTargetView(pRenderTargetView, D3DXCOLOR(0.5, 0.5, 0.5, 1));
    pD3DDevice->ClearDepthStencilView(pDepthStencilView, D3D10_CLEAR_DEPTH, 1.0f, 0);

    D3DXMATRIX scaling;
    D3DXMatrixScaling(&scaling, 1.5, 1.5, 1.5);

    //draw
    for (UINT p = 0; p < techDesc.Passes; p++)
    {
        //set resources for plane 1
        D3DXMatrixTranslation(&worldMatrix, 0, 0, 2);
        worldMatrix = scaling * worldMatrix;
        pWorldMatrixEffectVariable->SetMatrix(worldMatrix);
        pTextureSR->SetResource(pDiffuseTex);

        //apply technique
        pBasicTechnique->GetPassByIndex(p)->Apply(0);
        pCubeMesh->DrawSubset(0);

        //set resources for plane 2
        D3DXMatrixTranslation(&worldMatrix, 0, 0, 1.5);
        pWorldMatrixEffectVariable->SetMatrix(worldMatrix);
        pTextureSR->SetResource(pAlphaTex);

        //apply technique
        pBasicTechnique->GetPassByIndex(p)->Apply(0);
        pCubeMesh->DrawSubset(0);
    }

    //flip buffers
    pSwapChain->Present(0, 0);
}

/*******************************************************************
* Fatal Error Handler
*******************************************************************/
bool dxManager::fatalError(const LPCSTR msg)
{
    MessageBoxA(*hWnd, msg, "Fatal Error!", MB_ICONERROR);
    return false;
}