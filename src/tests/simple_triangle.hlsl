#pragma enable_d3d11_debug_symbols

cbuffer CameraBuffer : register(b0)
{
    float4x4 view;
    float4x4 projection;
};

cbuffer ObjectBuffer : register(b1)
{
    float4x4 world;
};

struct VSInput
{
    float3 position : POSITION;
};

struct VSOutput
{
    float4 position : SV_POSITION;
};

VSOutput VSMain(VSInput input)
{
    VSOutput o;

    o.position = mul(projection, mul(view, mul(world, float4(input.position, 1.0))));
    return o;
}

float4 PSMain() : SV_TARGET
{
    return float4(0.0, 1.0, 0.0, 1.0);
}
