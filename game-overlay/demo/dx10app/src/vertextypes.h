#pragma once
//create a basic vertex type
struct vertex
{
    D3DXVECTOR3 pos;
    D3DXVECTOR4 color;
    D3DXVECTOR2 texCoord;

    vertex()
    {
    }

    vertex(D3DXVECTOR3 p, D3DXVECTOR4 c)
    {
        pos = p;
        color = c;
        texCoord = D3DXVECTOR2(0, 0);
    }

    vertex(D3DXVECTOR3 p, D3DXVECTOR4 c, D3DXVECTOR2 t)
    {
        pos = p;
        color = c;
        texCoord = t;
    }
};

const int numInputLayoutElements = 3;

const D3D10_INPUT_ELEMENT_DESC vertexInputLayout[] =
{
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },
{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D10_INPUT_PER_VERTEX_DATA, 0 },
{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D10_INPUT_PER_VERTEX_DATA, 0 }
};