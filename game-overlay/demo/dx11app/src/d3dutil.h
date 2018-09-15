#pragma once

#ifndef HRERROR
#define HRERROR(x)                                              \
        do \
        {\
            HRESULT hr = (x);                                      \
            if (FAILED(hr))                                        \
                {                                                  \
                DXTrace(__FILE__, (DWORD)__LINE__, hr, L#x, true); \
                return false;                                      \
            }                                                      \
        } while (false);
#endif

#ifndef HR
#define HR(x)                                              \
        do \
        {\
            HRESULT hr = (x);                                      \
            if (FAILED(hr))                                        \
            {                                                  \
                DXTrace(__FILE__, (DWORD)__LINE__, hr, L#x, true); \
                return;                                      \
            }                                                      \
        } while (false);
#endif

#define ReleaseCOM(x) { if(x){ x->Release(); x = 0; } }


namespace Colors
{
    
    XMGLOBALCONST XMVECTORF32 White = { 1.0f, 1.0f, 1.0f, 1.0f };
    XMGLOBALCONST XMVECTORF32 Black = { 0.0f, 0.0f, 0.0f, 1.0f };
    XMGLOBALCONST XMVECTORF32 Red = { 1.0f, 0.0f, 0.0f, 1.0f };
    XMGLOBALCONST XMVECTORF32 Green = { 0.0f, 1.0f, 0.0f, 1.0f };
    XMGLOBALCONST XMVECTORF32 Blue = { 0.0f, 0.0f, 1.0f, 1.0f };
    XMGLOBALCONST XMVECTORF32 Yellow = { 1.0f, 1.0f, 0.0f, 1.0f };
    XMGLOBALCONST XMVECTORF32 Cyan = { 0.0f, 1.0f, 1.0f, 1.0f };
    XMGLOBALCONST XMVECTORF32 Magenta = { 1.0f, 0.0f, 1.0f, 1.0f };

    XMGLOBALCONST XMVECTORF32 Silver = { 0.75f, 0.75f, 0.75f, 1.0f };
    XMGLOBALCONST XMVECTORF32 LightSteelBlue = { 0.69f, 0.77f, 0.87f, 1.0f };

    XMGLOBALCONST XMVECTORF32 GroundGreen = { 33.f/255.f, 66.f/255.f, 0.f, 1.0f };

}


class Convert
{
public:
    ///<summary>
    /// Converts XMVECTOR to XMCOLOR, where XMVECTOR represents a color.
    ///</summary>
    static D3DX11INLINE XMCOLOR ToXmColor(FXMVECTOR v)
    {
        XMCOLOR dest;
        XMStoreColor(&dest, v);
        return dest;
    }

    ///<summary>
    /// Converts XMVECTOR to XMFLOAT4, where XMVECTOR represents a color.
    ///</summary>
    static D3DX11INLINE XMFLOAT4 ToXmFloat4(FXMVECTOR v)
    {
        XMFLOAT4 dest;
        XMStoreFloat4(&dest, v);
        return dest;
    }

    static D3DX11INLINE UINT ArgbToAbgr(UINT argb)
    {
        BYTE A = (argb >> 24) & 0xff;
        BYTE R = (argb >> 16) & 0xff;
        BYTE G = (argb >> 8) & 0xff;
        BYTE B = (argb >> 0) & 0xff;

        return (A << 24) | (B << 16) | (G << 8) | (R << 0);
    }

};


inline RECT calcFpsTexRect(ID3D11Texture2D* texture, int value)
{
    D3D11_TEXTURE2D_DESC desc;
    memset(&desc, 0, sizeof(desc));
    texture->GetDesc(&desc);

    int slice = 4;

    int sliceWidth = desc.Width / slice;
    int sliceHeight = desc.Height / slice;

    int vIndex = value / slice;
    int hIndex = value - slice*vIndex;


    RECT rc;
    rc.left = hIndex * sliceWidth;
    rc.top = vIndex * sliceHeight;
    rc.right = rc.left + sliceWidth;
    rc.bottom = rc.top + sliceHeight;

    return rc;
}

inline XMMATRIX matrixForZOrder(float z)
{
    XMMATRIX mat(
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, z, 1.0f
    );
    return mat;
}