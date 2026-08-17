
#ifndef __Camera_hpp__
#define __Camera_hpp__

#include "Common.hpp"

struct alignas(256) _BaseCamera
{
	float4 camera_pos;
	float4 camera_forward;
	float4 camera_right;
	float4 camera_up;

	float tanHalfFov;
	float aspect;
	float2 paddding;
};

class BaseCamera: public _BaseCamera {
public:
	virtual ~BaseCamera() = default;
	virtual void setPosition(float UNUSED_PARAM(x), float UNUSED_PARAM(x), float UNUSED_PARAM(z)) {};
	virtual void setLookAt(float UNUSED_PARAM(x), float UNUSED_PARAM(x), float UNUSED_PARAM(z)) {};
	virtual void setUp(float UNUSED_PARAM(x), float UNUSED_PARAM(x), float UNUSED_PARAM(z)) {};
	virtual void setFOV(float UNUSED_PARAM(value)) {};
protected:
	BaseCamera() = default;
};
struct alignas(256)GeometryInstance
{
	uint vertex_resource_id;
	uint index_resource_id;
	uint material_id;
	float4x4 world;
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
