#ifndef CAMERA_HLSL
#define CAMERA_HLSL

struct _BaseCamera
{
    float4 camera_pos;
    float4 camera_forward;
    float4 camera_right;
    float4 camera_up;

    float tanHalfFov;
    float aspect;
    float2 padding;
};
ConstantBuffer<_BaseCamera> g_camera : register(b0);

#endif