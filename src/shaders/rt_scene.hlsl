#ifndef RT_MAIN_HLSL
#define RT_MAIN_HLSL

#define DRAW_EDGES

//#include "config.hlsl"
#include "common.hlsl"
#include "geometry.hlsl"
#include "sampler.hlsl"
//#include "bsdf.hlsl"
//#include "environment.hlsl"
//#include "material.hlsl"

///////////////////////////////////////////////////////////////////////////////
// Resources
///////////////////////////////////////////////////////////////////////////////



RaytracingAccelerationStructure scene_bvh : register(t0);
RWTexture2D<float4> render_surface : register(u0);

///////////////////////////////////////////////////////////////////////////////
// Ray Generation
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Generate Primary Ray
///////////////////////////////////////////////////////////////////////////////

float4 paint_pixel_with_edges(bool is_edge)
{
    float4 finalColor;
    if (is_edge)
        return float4(1.0, 0.0, 0.0, 1.0);
    else
        return float4(0.0, 1.0, 0.0, 1.0);
}

bool detect_edge(float2 barycentrics)
{
     // 1. Obtener las tres coordenadas baricéntricas del triángulo
    float u = barycentrics.x;
    float v = barycentrics.y;
    float w = 1.0f - u - v;

    // 2. Definir el grosor de la línea del wireframe (ajustable)
    float edgeThickness = 0.02f;

    // 3. Determinar si el rayo impactó cerca de un borde
    return (u < edgeThickness || v < edgeThickness || w < edgeThickness);
 }

[shader("raygeneration")]
void RT_RayGen()
{
    uint2 pixel = DispatchRaysIndex().xy;

    RayDesc ray =
        sampler_generate_rays(pixel);

    Payload payload;

    payload.hit = false;
    payload.distance = 0;
    payload.instance_index = 0;
    payload.primitive_index = 0;
    payload.barycentrics = 0;

    TraceRay(
        scene_bvh,
        RAY_FLAG_NONE,
        0xFF,
        0,
        1,
        0,
        ray,
        payload);

    float4 finalColor;

    if (payload.hit)
    {
        // Build geometry information from the hit.
        SurfaceData surface;

        build_surface_data(
            payload.instance_index,
            payload.primitive_index,
            surface);

        // Temporary coloring.
        finalColor = paint_pixel_with_edges(payload.is_edge);
    }
    else
    {
        finalColor = float4(0.1, 0.3, 0.8, 1.0);
    }

    render_surface[pixel] = finalColor;
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
    
    payload.is_edge = detect_edge(attribs.barycentrics);
    
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