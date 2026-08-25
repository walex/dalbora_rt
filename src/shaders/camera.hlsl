#ifndef CAMERA_HLSL
#define CAMERA_HLSL

#include "sampler.hlsl"

RayDesc camera_generate_rays(
    uint2 pixel, _BaseCamera camera, _SamplerConfig sampler_config)
{
    return sampler_generate_rays(pixel, camera, sampler_config);
}
#endif