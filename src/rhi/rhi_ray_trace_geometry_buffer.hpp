#ifndef __rhi_ray_trace_geometry_buffer_hpp__
#define __rhi_ray_trace_geometry_buffer_hpp__

#include "rhi_impl.hpp"

class RhiDevice;
class RhiView;
class RhiCommandBuffer;
class RhiGPUBuffer;
class RhiRayTraceGeometrydBuffer;
class RhiRayTraceGeometrydBufferInstances
	: public RhiImpl<RHI_BUFFER> {
public:
	RhiRayTraceGeometrydBufferInstances(RHI_BUFFER* buffer = nullptr);
	void create(const RhiDevice& device, const RhiCommandBuffer& command_buffer,
		RhiRayTraceGeometrydBuffer& geometry_buffer, const std::vector<float*>& instance_transforms);
	void update(const RhiDevice& device, const RhiCommandBuffer& command_buffer,
		RhiRayTraceGeometrydBuffer& geometry_buffer, const std::vector<float*>& instance_transforms);
};

class RhiRayTraceGeometrydBuffer 

	: public RhiImpl<RHI_RT_BVH>
	, public ICreateRhiObject<const RhiDevice&, const RhiCommandBuffer&,
	const std::vector<float*>&,
	const RhiGPUBuffer&, const RhiGPUBuffer*> {

public:
	RhiRayTraceGeometrydBuffer(RHI_RT_BVH* handle = nullptr);
	virtual ~RhiRayTraceGeometrydBuffer() = default;
	void create(const RhiDevice& device, const RhiCommandBuffer& command_buffer,
		const std::vector<float*>& instance_transforms,
		const RhiGPUBuffer& vertex_buffer, const RhiGPUBuffer* index_buffer = nullptr);
	void update(const RhiDevice& device, const RhiCommandBuffer& command_buffer, 
		const std::vector<float*>& instance_transforms);
	RhiView new_view(RhiDevice& device);
private:
	RhiRayTraceGeometrydBufferInstances m_geomtric_instances;
};

#endif // __rhi_ray_trace_geometry_buffer_hpp__
