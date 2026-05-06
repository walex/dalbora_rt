cbuffer Transform : register(b0)
{
    float4x4 rotation;
    float4 color;
};

struct VSInput
{
    float3 pos : POSITION;
};

struct VSOutput
{
    float4 pos : SV_POSITION;
};

VSOutput VSMain(VSInput input)
{
    VSOutput output;
    output.pos = mul(rotation, float4(input.pos, 1.0));
    return output;
}

float4 PSMain() : SV_TARGET
{
    return color;
}
