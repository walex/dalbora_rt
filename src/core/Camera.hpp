
#ifndef __Camera_hpp__
#define __Camera_hpp__

#include "Common.hpp"

struct alignas(256) _BaseCamera
{
	float4 camera_pos;
	float4 camera_forward;
	float4 camera_right;
	float4 camera_up;

	float tan_half_fov;
	float aspect;
	float2 paddding;
};

class BaseCamera: protected _BaseCamera {
public:

	IMPLEMENT_COPYABLE_AND_MOVABLE_CLASS(BaseCamera);

	virtual void set_position(float3 pos) { camera_pos = float4(pos, 1.0f); }
	virtual void set_look_at(float3 lookAt) { camera_forward = float4(normalize(lookAt), 0.0f); }
	virtual void set_up(float3 up) { camera_up = float4(normalize(up), 0.0f); }
	virtual void set_right(float3 right) { camera_right = float4(normalize(right), 0.0f); }
	virtual void set_fov(float value) { tan_half_fov = value; }
	virtual void set_aspect(float value) { aspect = value; }
	virtual void update(float dt);
	virtual ~BaseCamera();
	BaseCamera(RhiDevice& device);
private:
	RhiSharedBuffer m_transforms;
	RhiView m_transforms_view;
	RhiSharedBufferMap m_buffer_map;
};

class Sampler;
class Samples;
class Rays;
class Camera : public BaseCamera
{
public:
	virtual ~Camera() = default;
	void setSampler(const Sampler* const sampler) { m_sampler = sampler; }
	Samples generateSamples(const size_t grid[4]) const;
	
private: 
	const Sampler* m_sampler;
};

#endif
