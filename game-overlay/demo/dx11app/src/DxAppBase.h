/*
    Beginning DirectX 11 Game Programming
    By Allen Sherrod and Wendy Jones

    DirectX 11 Base Class - Used as base class for all DirectX 11 demos in this book.
    */


#ifndef _DEMO_BASE_H_
#define _DEMO_BASE_H_


class DxAppBase
{
public:
    DxAppBase();
    virtual ~DxAppBase();

    bool Init(HINSTANCE instance);

    bool InitWindow();
    bool InitDirect3D(HWND window);

    void ShowGameWindow();

    void Shutdown();

    int RunLoop();

    virtual void initRenderState();

    virtual bool LoadContent();
    virtual void UnloadContent();

    virtual void UpdateScene(float dt);
    virtual void DrawScene();

    void PresentScene();

    void CalcFPS();

    SIZE renderTargetSize();

    void enableAlpha();
    void restoreAlpha();

    virtual LRESULT WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual void OnMouseDown(WPARAM btnState, int x, int y);
    virtual void OnMouseUp(WPARAM btnState, int x, int y);
    virtual void OnMouseMove(WPARAM btnState, int x, int y);

protected:
    virtual void OnResize();

    virtual bool loadFPS();
    virtual void unloadFPS();
    virtual void drawFPS();

protected:
    HINSTANCE mInstance;
    HWND mWindow;
    GameTimer mTimer;
    std::wstring m_gpudescription;

    int m_fps;

    bool mAppPaused;
    int mClientWidth;
    int mClientHeight;

    Camera mCamera;
    POINT mLastMousePos;


    D3D_DRIVER_TYPE mDriverType;
    D3D_FEATURE_LEVEL mFeatureLevel;

    bool m4xMsaaEnable;
    UINT m4xMsaaQuality;

    ID3D11Device* md3dDevice;
    ID3D11DeviceContext* md3dContext;

    IDXGISwapChain* mSwapChain;
    ID3D11RenderTargetView* mRenderTargetView;

    ID3D11Texture2D* mDepthStencilBuffer;
    ID3D11DepthStencilView* mDepthStencilView;

    Windows::ComPtr<ID3D11BlendState> pTransparentBS;

    Windows::ComPtr<ID3D11BlendState> pOpaqueBS;

    struct SavedBS
    {
        Windows::ComPtr<ID3D11BlendState> BS;
        float blen_factor[4];
        UINT blen_mask;
    } savedBS;
    
    Windows::ComPtr<ID3D11Texture2D> m_fpsTexture;
    Windows::ComPtr<ID3D11ShaderResourceView> m_fpstextureView;

    std::unique_ptr<DX11Sprite> m_spriteDrawer;

    Windows::ComPtr<IFW1Factory > m_fw1FontFactory;
    Windows::ComPtr<IFW1FontWrapper > m_fwFontWrapper;
};

#endif