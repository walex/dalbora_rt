
#ifndef __Camera_hpp__
#define __Camera_hpp__

#include "Common.hpp"

struct alignas(256) CameraBuffer
{
	float4 camera_pos;
	float4 camera_forward;
	float4 camera_right;
	float4 camera_up;

	float tan_half_fov;
	float aspect;
	float pad0;
	float pad1;
};

class ResourceManager;
class BaseCamera: public CameraBuffer {
public:

	IMPLEMENT_MOVABLE_CLASS(BaseCamera);

	virtual void set_position(float3 pos) { this->camera_pos = float4(pos, 1.0f); }
	virtual void set_look_at(float3 lookAt) { this->camera_forward = float4(normalize(lookAt), 0.0f); }
	virtual void set_up(float3 up) { this->camera_up = float4(normalize(up), 0.0f); }
	virtual void set_right(float3 right) { this->camera_right = float4(normalize(right), 0.0f); }
	virtual void set_fov(float value) { this->tan_half_fov = value; }
	virtual void set_aspect(float value) { this->aspect = value; }
	virtual void update(float dt);
	virtual ~BaseCamera();
	BaseCamera(ResourceManager& rn);
private:
	RhiSharedBuffer m_transforms;
	std::unique_ptr<RhiSharedBufferMap> m_buffer_map;
	RhiView m_camera_transform_view;
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
