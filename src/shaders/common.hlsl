#ifndef COMMON_HLSL
#define COMMON_HLSL

//=============================================================================
// Constants
//=============================================================================

static const float PI      = 3.14159265358979323846f;
static const float TWO_PI  = 6.28318530717958647692f;
static const float INV_PI  = 0.31830988618379067154f;
static const float EPSILON = 1e-4f;

#ifndef FLT_MAX
#define FLT_MAX 3.402823466e+38f
#endif

#define INVALID_TEXTURE    0xffffffff
#define INVALID_SAMPLER    0xffffffff

//=============================================================================
// Camera
//=============================================================================

struct CameraData
{
    float4x4 View;
    float4x4 Projection;

    float4x4 InvView;
    float4x4 InvProjection;

    float3 Position;
    float NearPlane;

    float3 Forward;
    float FarPlane;

    uint FrameIndex;
    uint SamplesPerPixel;
    uint MaxDepth;
    uint Flags;

    float Exposure;
    float Gamma;

    float2 Padding;
};

//=============================================================================
// Ray Payload
//=============================================================================
struct [raypayload] Payload
{
    bool hit
        : read(caller)
        : write(caller, closesthit, miss);

    bool is_edge
        : read(caller)
        : write(caller, closesthit);

    uint instance_index
            : read(caller)
            : write(caller, closesthit);

    uint primitive_index
            : read(caller)
            : write(caller, closesthit);

    float2 barycentrics
            : read(caller)
            : write(caller, closesthit);

    float distance
            : read(caller)
            : write(caller, closesthit);

    bool front_face 
            : read(caller)
            : write(caller, closesthit);

    
};

//=============================================================================
// Ray State
//
// Estado del path tracer.
// Nunca viaja por TraceRay().
//=============================================================================

struct RayState
{
    RayDesc ray;

    float3 throughput;

    float3 radiance;

    uint depth;

    uint randomState;
};

//=============================================================================
// ClosestHit attributes
//=============================================================================

struct Attributes
{
    float2 barycentrics;
};

//=============================================================================
// Material
//
//=============================================================================

struct Material
{
    float4 baseColorFactor;

    float metallicFactor;
    float roughnessFactor;

    uint baseColorTexture;
    uint metallicRoughnessTexture;

    uint normalTexture;
    uint emissiveTexture;

    float3 emissiveFactor;

    uint samplerIndex;
};


//=============================================================================
// Surface Geometry
//
// Resultado de reconstruir la geometría.
//=============================================================================

struct SurfaceGeometry
{
    float3 position;

    float3 normal;

    float3 tangent;

    float3 bitangent;

    float2 texcoord;

    uint materialIndex;
};


struct SurfaceInteraction
{
    float3 position;

    float3 normal;

    float3 albedo;

    float metallic;

    float roughness;

    float3 emissive;
};

#endif