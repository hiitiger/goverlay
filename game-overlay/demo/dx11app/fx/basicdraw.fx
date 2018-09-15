
cbuffer PerFrame
{
    float4x4	g_worldViewProj;
};

struct VOut
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

struct VertexInputType
{
    float4 position : POSITION;
    float4 color : COLOR;
};

VOut VShader(VertexInputType input)
{
    VOut output;
    input.position.w = 1.0f;
    output.position = mul(input.position, g_worldViewProj);
    output.color = input.color;

    return output;
}


float4 PShader(float4 position : SV_POSITION, float4 color : COLOR) : SV_TARGET
{
    return color;
}

technique11 BasicDraw
{
    Pass p0
    {
        SetVertexShader(CompileShader(vs_5_0, VShader()));
        SetPixelShader(CompileShader(ps_5_0, PShader()));
    }
}
