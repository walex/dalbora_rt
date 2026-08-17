#ifndef GEOMETRY_HLSL
#define GEOMETRY_HLSL

#include "common.hlsl"
#include "linear_algebra.hlsl"

///////////////////////////////////////////////////////////////////////////////
// GPU Geometry Structures
//
// Estas estructuras deben coincidir con los buffers creados desde C++.
//
///////////////////////////////////////////////////////////////////////////////

struct GeometryVertex
{
    float3 position;
    float3 normal;
    float4 tangent;
    float2 uv;
};


struct GeometryPrimitive
{
    uint index_offset; // Usually 0 unless shared sub-mesh
    uint vertex_offset; // Usually 0
    uint material_index;
};


struct GeometryInstance
{
    uint vertex_resource_id;
    uint index_resource_id;
    uint material_id;
    float4x4 world;
};

struct SurfaceData
{
    GeometryVertex v0;
    GeometryVertex v1;
    GeometryVertex v2;

    float3 n0;
    float3 n1;
    float3 n2;

    uint material_id;
    
};

///////////////////////////////////////////////////////////////////////////////
// Geometry Buffers
///////////////////////////////////////////////////////////////////////////////

// Culled active instances
StructuredBuffer<GeometryInstance> gGeometryInstances : register(t1);


// Fixed metadata for all primitives in the scene
StructuredBuffer<GeometryPrimitive> gGeometryPrimitives
    : register(t2);


// Global bindless array of all vertices in the scene
StructuredBuffer<GeometryVertex> gGeometryVertices[]
    : register(t3);

// Global bindless array of all indices in the scene
StructuredBuffer<uint> gGeometryIndices[]
    : register(t4);

inline GeometryInstance get_geometry_instance(uint instance_id)
{
    return gGeometryInstances[instance_id];
}

inline StructuredBuffer<GeometryVertex> get_vertex_buffer(GeometryInstance instance)
{
    return gGeometryVertices[ instance.vertex_resource_id ];
}

inline StructuredBuffer<uint> get_index_buffer(GeometryInstance instance)
{
    return gGeometryIndices[ instance.index_resource_id ];
}

inline GeometryPrimitive get_geometry_primitive(uint instance_id, uint primitive_id)
{
    GeometryInstance instance = get_geometry_instance(instance_id);
    return gGeometryPrimitives[instance.material_id + primitive_id];
}

inline void build_surface_data(
    uint instance_id,
    uint primitive_id,
    inout SurfaceData surface)
{
    GeometryInstance instance =
        get_geometry_instance(instance_id);

    StructuredBuffer<GeometryVertex> vertices =
        get_vertex_buffer(instance);

    StructuredBuffer<uint> indices =
        get_index_buffer(instance);

    uint i0 = indices[primitive_id * 3 + 0];
    uint i1 = indices[primitive_id * 3 + 1];
    uint i2 = indices[primitive_id * 3 + 2];

    surface.v0 = vertices[i0];
    surface.v1 = vertices[i1];
    surface.v2 = vertices[i2];

    float3x3 normalMatrix =
    InverseTranspose3x3((float3x3) instance.world);

    surface.n0 = normalize(mul(surface.v0.normal, normalMatrix));
    surface.n1 = normalize(mul(surface.v1.normal, normalMatrix));
    surface.n2 = normalize(mul(surface.v2.normal, normalMatrix));
}

/*
///////////////////////////////////////////////////////////////////////////////
// Load Vertex
///////////////////////////////////////////////////////////////////////////////

GPUVertex LoadVertex(uint index)
{
    return Vertices[index];
}



///////////////////////////////////////////////////////////////////////////////
// Reconstruct Surface
///////////////////////////////////////////////////////////////////////////////

void LoadGeometry(
    Payload payload,
    out SurfaceGeometry geometry)
{

    GPUInstance instance =
        Instances[payload.instanceIndex];


    GPUPrimitive primitive =
        Primitives[
            instance.primitiveOffset +
            payload.primitiveIndex
        ];


    //
    // Obtener índices del triángulo
    //

    uint i0 =
        Indices[
            primitive.indexOffset + 0
        ];

    uint i1 =
        Indices[
            primitive.indexOffset + 1
        ];

    uint i2 =
        Indices[
            primitive.indexOffset + 2
        ];


    GPUVertex v0 =
        LoadVertex(
            primitive.vertexOffset + i0);


    GPUVertex v1 =
        LoadVertex(
            primitive.vertexOffset + i1);


    GPUVertex v2 =
        LoadVertex(
            primitive.vertexOffset + i2);



    //
    // Barycentric interpolation
    //

    float u =
        payload.barycentrics.x;

    float v =
        payload.barycentrics.y;

    float w =
        1.0f - u - v;



    float3 position =
          v0.position * w
        + v1.position * u
        + v2.position * v;


    float3 normal =
        normalize(
              v0.normal * w
            + v1.normal * u
            + v2.normal * v);



    float4 tangent =
          v0.tangent * w
        + v1.tangent * u
        + v2.tangent * v;



    float2 uv =
          v0.texcoord * w
        + v1.texcoord * u
        + v2.texcoord * v;



    //
    // Transformación a world space
    //
    // IMPORTANTE:
    //
    // En DXR la instancia del TLAS tiene una transformación.
    //
    // Aquí luego agregaremos:
    //
    // ObjectToWorld(instanceIndex)
    //
    // cuando definamos el buffer de transforms.
    //



    geometry.position =
        position;


    geometry.normal =
        normalize(normal);


    geometry.tangent =
        normalize(tangent.xyz);


    geometry.bitangent =
        normalize(
            cross(
                geometry.normal,
                geometry.tangent)
            *
            tangent.w);


    geometry.texcoord =
        uv;


    geometry.materialIndex =
        primitive.materialIndex;
}


*/

#endif