#ifndef __Spatial_hpp__
#define __Spatial_hpp__

#include "Common.hpp"

class Spatial {

public:
	virtual ~Spatial() = default;
	const float4x4& get_world_transform() const { return m_world_transform; }
	const float4x4& get_local_transform() const { return m_local_transform; }
	void set_world_transform(const float4x4& transform) { m_world_transform = transform; }
	void set_local_transform(const float4x4& transform) { m_local_transform = transform; m_is_local_transform_dirty = true; }
protected:
	Spatial() = default;
	float4x4 m_world_transform = float4x4::Identity();
	float4x4 m_local_transform = float4x4::Identity();
	bool m_is_local_transform_dirty = true;
};

#endif
