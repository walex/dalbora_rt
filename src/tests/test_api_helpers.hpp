#ifndef __test_api_helpers_hpp__
#define __test_api_helpers_hpp__

#include "C:\Users\wadrw\Documents\develop\projects\personal\rtx\MathLib\ml.h"
constexpr float XM_PI = 3.141592654f;
constexpr float XMConvertToRadians(float fDegrees) noexcept { return fDegrees * (XM_PI / 180.0f); }
constexpr float XMConvertToDegrees(float fRadians) noexcept { return fRadians * (180.0f / XM_PI); }
constexpr float XM_PIDIV4 = 0.785398163f;

struct alignas(256) CameraCB
{
	float4x4 view;
	float4x4 projection;
};
struct alignas(256) ObjectCB
{
	float4x4 world = float4x4::Identity();
};
struct alignas(256) CameraCBRT
{
	float4 camera_pos;
	float4 camera_forward;
	float4 camera_right;
	float4 camera_up;

	float tanHalfFov;
	float aspect;    
	float pad0;      
	float pad1;      
};

inline void get_transforms(float4x4& world, float4x4& view, float4x4& projection)
{

	world = float4x4::Identity();

	float3 camera_pos =
		float3(0.0f, 0.0f, 3.0f);

	float3 camera_forward =
		float3(0.0f, 0.0f, 1.0f);

	float3 camera_right =
		float3(1.0f, 0.0f, 0.0f);

	float3 camera_up =
		normalize(cross(camera_forward, camera_right));

	view.ca[0] = float4(
		camera_right.x,
		camera_right.y,
		camera_right.z,
		0.0f);

	view.ca[1] = float4(
		camera_up.x,
		camera_up.y,
		camera_up.z,
		0.0f);

	view.ca[2] = float4(
		camera_forward.x,
		camera_forward.y,
		camera_forward.z,
		0.0f);

	view.ca[3] = float4(
		camera_pos.x,
		camera_pos.y,
		camera_pos.z,
		1.0f);

	projection.SetupByHalfFovy(XM_PIDIV4*0.5f, 800.0f / 600.0f, 0.1f, 100.0f, PROJ_LEFT_HANDED);

}

inline float get_delta_time()
{
	using clock = std::chrono::high_resolution_clock;

	static auto previous_time = clock::now();

	auto current_time = clock::now();

	std::chrono::duration<float> delta =
		current_time - previous_time;

	previous_time = current_time;

	return delta.count();
}

inline float4x4 rotate_triangle(float dt) {

	static float angle = 0.0f;
	static float speed =
		XMConvertToRadians(85.0f);

	float4x4 rotation;
	rotation.SetupByRotationY(angle);

	angle += dt * speed;
	return rotation;
}

#endif