#ifndef RT_MAIN_HLSL
#define RT_MAIN_HLSL

#define DRAW_EDGES

//#include "config.hlsl"
#include "common.hlsl"
//#include "geometry.hlsl"
//#include "bsdf.hlsl"
//#include "environment.hlsl"
//#include "material.hlsl"

///////////////////////////////////////////////////////////////////////////////
// Resources
///////////////////////////////////////////////////////////////////////////////

struct CameraCB
{
    float4 camera_pos;
    float4 camera_forward;
    float4 camera_right;
    float4 camera_up;

    float tanHalfFov;
    float aspect;
    float2 padding;
};

struct GeomrtryInstance
{
    uint vertex_resource_id;
    uint index_resource_id;
    uint material_id;
    float4x4 world;
};

RaytracingAccelerationStructure SceneBVH : register(t0);
StructuredBuffer<GeomrtryInstance> gInstances : register(t1);

RWTexture2D<float4> Output : register(u0);
ConstantBuffer<CameraCB> Camera : register(b0);

///////////////////////////////////////////////////////////////////////////////
// Ray Generation
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Generate Primary Ray
///////////////////////////////////////////////////////////////////////////////


RayDesc GeneratePrimaryRay(
    uint2 pixel,
    float2 jitter)
{
    uint2 resolution = DispatchRaysDimensions().xy;

    float2 uv =
        (float2(pixel) + jitter) /
        float2(resolution);

    // pasar de [0,1] a [-1,1]
    uv = uv * 2.0f - 1.0f;

    // corregir eje Y de imagen
    uv.y = -uv.y;

    RayDesc ray;

    float tanHalfFov = Camera.tanHalfFov;
    float aspect = Camera.aspect;

    float3 direction =
        Camera.camera_forward.xyz +
        uv.x * aspect * tanHalfFov * Camera.camera_right.xyz +
        uv.y * tanHalfFov * Camera.camera_up.xyz;

    ray.Origin = Camera.camera_pos.xyz;
    ray.Direction = normalize(direction);

    ray.TMin = EPSILON;
    ray.TMax = FLT_MAX;

    return ray;
}

[shader("raygeneration")]
void RT_RayGen()
{
    uint2 pixel = DispatchRaysIndex().xy;
	
	RayDesc ray =
    GeneratePrimaryRay(
        pixel,
        float2(0.5, 0.5));
		
	Payload payload;

	payload.hit = false;
	payload.distance = 0;
	payload.instance_index = 0;
	payload.primitive_index = 0;
	payload.barycentrics = 0;

	TraceRay(
		SceneBVH,
		RAY_FLAG_NONE,
		0xFF,
		0,
		1,
		0,
		ray,
		payload);
		
	float4 finalColor;	
	if(payload.hit) {
        if (payload.is_edge)
        {
            finalColor = float4(1.0, 0.0, 0.0, 1.0);
        }
        else
        {
            finalColor = float4(0.0, 1.0, 0.0, 1.0);
        }
	} else {
		finalColor = float4(0.1, 0.3, 0.8, 1.0);
	}
	Output[pixel] = finalColor;
}


///////////////////////////////////////////////////////////////////////////////
// Closest Hit
///////////////////////////////////////////////////////////////////////////////

[shader("closesthit")]
void RT_ClosestHit(
    inout Payload payload,
    in Attributes attribs)
{
    payload.hit = true;
    
  #ifdef DRAW_EDGES
     // 1. Obtener las tres coordenadas baricéntricas del triángulo
    float u = attribs.barycentrics.x;
    float v = attribs.barycentrics.y;
    float w = 1.0f - u - v;

    // 2. Definir el grosor de la línea del wireframe (ajustable)
    float edgeThickness = 0.02f;

    // 3. Determinar si el rayo impactó cerca de un borde
    if (u < edgeThickness || v < edgeThickness || w < edgeThickness)
    {
        payload.is_edge = true;
    }
    else
    {
        payload.is_edge = false;
    }
#else
    payload.is_edge = false;
#endif
    
    payload.instance_index = InstanceIndex();
    payload.primitive_index = PrimitiveIndex();
    payload.barycentrics = attribs.barycentrics;
    payload.distance = RayTCurrent();
    payload.front_face = HitKind() == HIT_KIND_TRIANGLE_FRONT_FACE;
}

///////////////////////////////////////////////////////////////////////////////
// Miss
///////////////////////////////////////////////////////////////////////////////

[shader("miss")]
void RT_Miss(
    inout Payload payload)
{
    payload.hit = false;
    payload.is_edge = false;
}

#endif