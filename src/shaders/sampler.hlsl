#ifndef SAMPLER_HLSL
#define SAMPLER_HLSL

#include "camera.hlsl"
#include "common.hlsl"

struct _SamplerConfig
{
    uint depth;
    float2 jitter;
    float padding; // 16 bytes alignment
};
ConstantBuffer<_SamplerConfig> g_samplerConfig : register(b1);

RayDesc sampler_generate_rays(
    uint2 pixel)
{
    uint2 resolution = DispatchRaysDimensions().xy;
    
	// TODO: replace with g_samplerConfig.jitter
	float2 jitter = float2(0.5,0.5);
     float2 uv =
        (float2(pixel) + jitter) /
        float2(resolution);

    // pasar de [0,1] a [-1,1]
    uv = uv * 2.0f - 1.0f;

    // corregir eje Y de imagen
    uv.y = -uv.y;

    RayDesc ray;

    float tanHalfFov = g_camera.tanHalfFov;
    float aspect = g_camera.aspect;

    float3 direction =
        g_camera.camera_forward.xyz +
        uv.x * aspect * tanHalfFov * g_camera.camera_right.xyz +
        uv.y * tanHalfFov * g_camera.camera_up.xyz;

    ray.Origin = g_camera.camera_pos.xyz;
    ray.Direction = normalize(direction);

    ray.TMin = EPSILON;
    ray.TMax = FLT_MAX;

    return ray;
}

#endif