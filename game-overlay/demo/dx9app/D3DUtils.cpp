#include "StdAfx.h"
#include "D3DUtils.h"

namespace D3DUtils{

    D3DMATERIAL9 initMaterial(D3DXCOLOR a, D3DXCOLOR d, D3DXCOLOR s, D3DXCOLOR e, float p)
    {
        D3DMATERIAL9 m;
        m.Ambient = a;
        m.Diffuse = d;
        m.Specular = s;
        m.Emissive = e;
        m.Power = p;
        return m;
    }

    D3DLIGHT9 initDirectionLight(D3DXVECTOR3* direction, const D3DXCOLOR* color)
    {
        D3DLIGHT9 light ;
        ::ZeroMemory(&light, sizeof(light));

        light.Type = D3DLIGHT_DIRECTIONAL;
        light.Ambient = *color * 0.4f;
        light.Diffuse = *color;
        light.Specular = *color * 0.6f;
        light.Direction = *direction;

        return light;
    }

    D3DLIGHT9 initPointLight(D3DXVECTOR3* direction, const D3DXCOLOR* color)
    {
        D3DLIGHT9 light ;
        ::ZeroMemory(&light, sizeof(light));

        light.Type = D3DLIGHT_POINT;
        light.Ambient = *color * 0.4f;
        light.Diffuse = *color;
        light.Specular = *color * 0.6f;
        light.Direction = *direction;

        return light;
    }

    D3DLIGHT9 initSpotLight(D3DXVECTOR3* direction, const D3DXCOLOR* color)
    {
        D3DLIGHT9 light ;
        ::ZeroMemory(&light, sizeof(light));

        light.Type = D3DLIGHT_SPOT;
        light.Ambient = *color * 0.4f;
        light.Diffuse = *color;
        light.Specular = *color * 0.6f;
        light.Direction = *direction;

        return light;
    }

    BoundingBox::BoundingBox()
    {
        // infinite small 
        _min.x = INFINITY;
        _min.y = INFINITY;
        _min.z = INFINITY;

        _max.x = -INFINITY;
        _max.y = -INFINITY;
        _max.z = -INFINITY;
    }

    bool BoundingBox::isPointInside(D3DXVECTOR3& p)
    {
        if (p.x >= _min.x && p.y >= _min.y && p.z >= _min.z &&
            p.x <= _max.x && p.y <= _max.y && p.z <= _max.z)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    BoundingSphere::BoundingSphere()
    {
        _radius = 0.0f;
    }

}
