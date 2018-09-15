#include "lighthelper.fx"


cbuffer MatrixBuffer
{
    matrix worldMatrix;
    matrix worldInvTranspose;
    matrix viewMatrix;
    matrix projectionMatrix;

};

cbuffer cbPerFrame
{
    DirLight gDirLight;
    PointLight gPointLight;
    SpotLight gSpotLight;
    float3 cameraPosition;
};

cbuffer cbPerObject
{
    Material gMaterial;
};

struct VertexInputType
{
    float4 position : POSITION;
    float3 normal : NORMAL;

};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float4 worldposition: POSITION;
    float3 normal : NORMAL;
};
////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PixelInputType LightVertexShader(VertexInputType input)
{
    PixelInputType output;

    // Change the position vector to be 4 units for proper matrix calculations.
    input.position.w = 1.0f;

    // Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    output.normal = mul(input.normal, (float3x3) worldInvTranspose);

    output.worldposition = mul(input.position, worldMatrix);

    return output;
}

float4 LightPixelShader(PixelInputType input) : SV_TARGET
{
    float4 color;

    float4 ambient = { 0.f, 0.f, 0.f, 0.f };
    float4 diffuse = { 0.f, 0.f, 0.f, 0.f };
    float4 specular = { 0.f, 0.f, 0.f, 0.f };

    float3 viewDirection;

    input.normal = normalize(input.normal);

    // Determine the viewing direction based on the position of the camera and the position of the vertex in the world.
    viewDirection = cameraPosition.xyz - input.worldposition.xyz;

    // Normalize the viewing direction vector.
    viewDirection = normalize(viewDirection);

    ComputeDirLight(gMaterial, gDirLight, input.normal, viewDirection, ambient, diffuse, specular);

    color = saturate(ambient + diffuse + specular);

    ComputePointLight(gMaterial, gPointLight, input.worldposition.xyz, input.normal, viewDirection, ambient, diffuse, specular);
    
    color = saturate(color + (ambient + diffuse + specular));

    ComputeSpotLight(gMaterial, gSpotLight, input.worldposition.xyz, input.normal, viewDirection, ambient, diffuse, specular);
    
    color = saturate(color + (ambient + diffuse + specular));
    
    color.a = gMaterial.diffuse.a;

    return color;
}


technique11 LightingDraw
{
    Pass p0
    {
        SetVertexShader(CompileShader(vs_5_0, LightVertexShader()));
        SetPixelShader(CompileShader(ps_5_0, LightPixelShader()));
    }
}
