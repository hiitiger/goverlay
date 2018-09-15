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

Texture2D image;

SamplerState sampledef
{
    Filter = MIN_MAG_MIP_LINEAR;

    AddressU = WRAP;
    AddressV = WRAP;
};

struct VertexInputType
{
    float4 position : POSITION;
    float3 normal : NORMAL;
    float2 tex : TEXCOORD;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float4 worldposition : POSITION;
    float3 normal : NORMAL;
    float2 tex : TEXCOORD;
};

PixelInputType TextureVShader(VertexInputType input)
{
    PixelInputType output;

    input.position.w = 1.0f;
   
    // Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    output.normal = mul(input.normal, (float3x3) worldInvTranspose);

    output.worldposition = mul(input.position, worldMatrix);

    output.tex = input.tex;

    return output;
}

float4 TexturePShader(PixelInputType input): SV_TARGET
{
    float4 texColor = float4(0, 0, 0, 0);
    texColor = image.Sample(sampledef, input.tex);
    return texColor;
}

float4 TexturePShaderWithLight(PixelInputType input) : SV_TARGET
{
    float4 lightColor = { 0.f, 0.f, 0.f, 0.f };
    float4 texColor = float4(1, 1, 1, 1);

    float4 ambient = { 0.f, 0.f, 0.f, 0.f };
    float4 diffuse = { 0.f, 0.f, 0.f, 0.f };
    float4 specular = { 0.f, 0.f, 0.f, 0.f };

    float4 A, D, S;

    float3 viewDirection;

    input.normal = normalize(input.normal);

    // Determine the viewing direction based on the position of the camera and the position of the vertex in the world.
    viewDirection = cameraPosition.xyz - input.worldposition.xyz;

    // Normalize the viewing direction vector.
    viewDirection = normalize(viewDirection);

    ComputeDirLight(gMaterial, gDirLight, input.normal, viewDirection, A, D, S);
    ambient += A;
    diffuse += D;
    specular += S;

    ComputePointLight(gMaterial, gPointLight, input.worldposition.xyz, input.normal, viewDirection, A, D, S);
    ambient += A;
    diffuse += D;
    specular += S;

    ComputeSpotLight(gMaterial, gSpotLight, input.worldposition.xyz, input.normal, viewDirection, A, D, S);
    ambient += A;
    diffuse += D;
    specular += S;
    

    texColor = image.Sample(sampledef, input.tex);
    
    lightColor = texColor * (ambient + diffuse) + specular;

    lightColor.a = gMaterial.diffuse.a * texColor.a;

    return lightColor;
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



technique11 TextureDrawWithLight
{
    Pass p0
    {
        SetVertexShader(CompileShader(vs_5_0, TextureVShader()));
        SetPixelShader(CompileShader(ps_5_0, TexturePShaderWithLight()));
    }
}


technique11 TextureDraw
{
    Pass p0
    {
        SetVertexShader(CompileShader(vs_5_0, TextureVShader()));
        SetPixelShader(CompileShader(ps_5_0, TexturePShader()));
    }
}


technique11 LightingDraw
{
    Pass p0
    {
        SetVertexShader(CompileShader(vs_5_0, TextureVShader()));
        SetPixelShader(CompileShader(ps_5_0, LightPixelShader()));
    }
}

