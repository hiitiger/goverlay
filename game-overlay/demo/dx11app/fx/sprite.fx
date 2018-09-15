Texture2D SpriteTex;
float4x4 transformMatrix;
    
SamplerState samLinear{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = WRAP;
    AddressV = WRAP;
};

struct VertexIn {
    float3 PosNdc : POSITION;
    float2 Tex : TEXCOORD;
    float4 Color : COLOR;
};

struct VertexOut {
    float4 PosNdc : SV_POSITION;
    float2 Tex : TEXCOORD;
    float4 Color : COLOR;
};

VertexOut VShader(VertexIn vin) {
    VertexOut vout;
    vout.PosNdc = mul(float4(vin.PosNdc, 1.0f), transformMatrix);
    vout.Tex = vin.Tex;
    vout.Color = vin.Color;
    return vout;
};

float4 PShader(VertexOut pin) : SV_Target{
    return pin.Color*SpriteTex.Sample(samLinear, pin.Tex);
};

technique11 SpriteTech {
pass P0{
    SetVertexShader(CompileShader(vs_5_0, VShader()));
    SetHullShader(NULL);
    SetDomainShader(NULL);
    SetGeometryShader(NULL);
    SetPixelShader(CompileShader(ps_5_0, PShader()));
    }
}