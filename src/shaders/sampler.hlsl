#ifndef SAMPLER_HLSL
#define SAMPLER_HLSL

#include "common.hlsl"

RayDesc sampler_generate_rays(
    uint2 pixel, _BaseCamera camera, _SamplerConfig sampler_config)
{
    uint2 resolution = DispatchRaysDimensions().xy;
    
	// TODO: replace with sampler_config.jitter
	float2 jitter = float2(0.5,0.5);
     float2 uv =
        (float2(pixel) + jitter) /
        float2(resolution);

    // pasar de [0,1] a [-1,1]
    uv = uv * 2.0f - 1.0f;

    // corregir eje Y de imagen
    uv.y = -uv.y;

    RayDesc ray;

    float tanHalfFov = camera.tanHalfFov;
    float aspect = camera.aspect;

    float3 direction =
        camera.camera_forward.xyz +
        uv.x * aspect * tanHalfFov * camera.camera_right.xyz +
        uv.y * tanHalfFov * camera.camera_up.xyz;

    ray.Origin = camera.camera_pos.xyz;
    ray.Direction = normalize(direction);

    ray.TMin = EPSILON;
    ray.TMax = FLT_MAX;

    return ray;
}

#endif