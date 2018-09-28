#pragma once


namespace D3DUtils
{

    const D3DXCOLOR WHITE = D3DXCOLOR(0xffffffff);
    const D3DXCOLOR BLACK = D3DXCOLOR(0xff000000);
    const D3DXCOLOR RED = D3DXCOLOR(0xffff0000);
    const D3DXCOLOR GREEN = D3DXCOLOR(0xff00ff00);
    const D3DXCOLOR BLUE = D3DXCOLOR(0xff0000ff);
    const D3DXCOLOR YELLOW = D3DXCOLOR(0xffffff00);


    //材质定义物体表面的反射属性
    D3DMATERIAL9 initMaterial(D3DXCOLOR a, D3DXCOLOR d, D3DXCOLOR s, D3DXCOLOR e, float p );

    const D3DMATERIAL9  WHITE_MTRL = initMaterial(WHITE, WHITE, WHITE, BLACK, 2.0f);
    const D3DMATERIAL9 RED_MTRL = initMaterial(RED, RED, RED, BLACK, 2.0f);
    const D3DMATERIAL9 GREEN_MTRL = initMaterial(GREEN, GREEN, GREEN, BLACK, 2.0f);
    const D3DMATERIAL9 BLUE_MTRL = initMaterial(BLUE, BLUE, BLUE, BLACK, 2.0f);
    const D3DMATERIAL9 YELLOW_MTRL = initMaterial(YELLOW, YELLOW, YELLOW, BLACK, 2.0f);

    D3DLIGHT9 initDirectionLight(D3DXVECTOR3* direction, const D3DXCOLOR* color);
    D3DLIGHT9 initPointLight(D3DXVECTOR3* direction, const D3DXCOLOR* color);
    D3DLIGHT9 initSpotLight(D3DXVECTOR3* direction, const D3DXCOLOR* color);



    struct BoundingBox
    {
        BoundingBox();

        bool isPointInside(D3DXVECTOR3& p);

        D3DXVECTOR3 _min;
        D3DXVECTOR3 _max;
    };

    struct BoundingSphere
    {
        BoundingSphere();

        D3DXVECTOR3 _center;
        float       _radius;
    };

    //
    // Constants
    //

    const float EPSILON = 0.001f;
};

