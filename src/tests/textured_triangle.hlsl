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

cbuffer SamplerConfig: register(b2) 
{
	float mipLevel;
};

struct VSInput
{
    float3 position : POSITION;
	float2 uv  : TEXCOORD;
};

struct VSOutput
{
    float4 position : SV_POSITION;
	float2 uv  : TEXCOORD;
};

VSOutput VSMain(VSInput input)
{
    VSOutput o;

    o.position = mul(projection, mul(view, mul(world, float4(input.position, 1.0))));
    o.uv = input.uv;

	return o;
}

struct PSInput
{
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD;
};

//cbuffer Material : register(b3)
//{
//    uint textureIndex;
//    uint samplerIndex;
//};

uint textureIndex = 0;
uint samplerIndex = 0;

Texture2D textures[] : register(t0, space0);
SamplerState samplers[] : register(s0, space0);

float4 PSMain(PSInput input) : SV_TARGET
{
 return textures[textureIndex].Sample(
    samplers[samplerIndex],
    input.uv);

}
