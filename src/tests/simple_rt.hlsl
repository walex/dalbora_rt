#pragma enable_d3d11_debug_symbols

struct RayPayload
{
    float4 color;
};

struct Attributes
{
    float2 barycentrics;
};

RWTexture2D<float4> Output : register(u0);
RaytracingAccelerationStructure SceneBVH : register(t0);

cbuffer CameraCB : register(b0)
{
    float3 camera_pos;
	float3 camera_forward;
	float3 camera_right;
	float3 camera_up;

	float tanHalfFov;
	float aspect;
	float2 padding;
};

[shader("raygeneration")]
void RayGen()
{

	uint2 pixel = DispatchRaysIndex().xy;
	uint2 dims  = DispatchRaysDimensions().xy;

	float2 uv =
		((float2(pixel) + 0.5)
		/ float2(dims))
		* 2.0 - 1.0;

	float3 ray_dir =
		normalize(
			camera_forward
		  + uv.x * aspect * tanHalfFov * camera_right
		  - uv.y * tanHalfFov * camera_up
		);
	
    RayDesc ray;
    ray.Origin = camera_pos;
    ray.Direction = ray_dir;
    ray.TMin = 0.001;
    ray.TMax = 10000.0;

    RayPayload payload;
    payload.color = float4(0, 0, 0, 1);

    TraceRay(
        SceneBVH,
        RAY_FLAG_NONE,
        0xFF,
        0, // ray contribution
        1, // multiplier
        0, // miss shader index
        ray,
        payload
    );

/////////// TEST //////////
/*
	RayQuery<RAY_FLAG_NONE> q;

    q.TraceRayInline(
        SceneBVH,
        RAY_FLAG_NONE,
        0xFF,
        ray);
*/
//	bool hit = q.Proceed();
/////////// TEST //////////

    Output[pixel] = payload.color;
	
}

[shader("miss")]
void Miss(inout RayPayload payload)
{
    payload.color = float4(0.1, 0.2, 0.4, 1.0);
}

[shader("closesthit")]
void ClosestHit(
    inout RayPayload payload,
    in Attributes attribs)
{
    payload.color = float4(0.0, 1.0, 0.0, 1.0);
}