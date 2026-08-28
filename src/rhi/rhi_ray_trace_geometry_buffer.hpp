#ifndef __rhi_ray_trace_geometry_buffer_hpp__
#define __rhi_ray_trace_geometry_buffer_hpp__

#include "rhi_buffer.hpp"

class RhiDevice;
class RhiView;
class RhiCommandBuffer;
class RhiGPUBuffer;
class RhiRayTraceGeometryBuffer;
class RhiRayTraceGeometrydBufferInstances
	: public RhiBuffer {
public:
	IMPLEMENT_MOVABLE_CLASS(RhiRayTraceGeometrydBufferInstances);

	RhiRayTraceGeometrydBufferInstances(RHI_BUFFER* buffer = nullptr);
	void create(const RhiDevice& device, const RhiCommandBuffer& command_buffer,
		const RhiRayTraceGeometryBuffer& geometry_buffer,
		const std::vector<float*>& instance_transforms);
	void create(const RhiDevice& device, const RhiCommandBuffer& command_buffer,
		const std::vector<RhiRayTraceGeometryBuffer>& geometry_buffer,
		const std::vector<std::vector<const float*>>& instance_transforms);
	void update(const RhiDevice& device, const RhiCommandBuffer& command_buffer,
		RhiRayTraceGeometryBuffer& geometry_buffer, const std::vector<float*>& instance_transforms);
	RhiView new_view(const RhiDevice& device);
private:
	void create(const RhiDevice& device, const size_t length,
		const size_t stride, const resource_format format = resource_format_none) override {}
};

class RhiRayTraceGeometryBuffer 

	: public RhiImpl<RHI_RT_BVH>
	, public ICreateRhiObject<const RhiDevice&, const RhiCommandBuffer&,
	const RhiGPUBuffer&, const RhiGPUBuffer*> {

public:
	IMPLEMENT_MOVABLE_CLASS(RhiRayTraceGeometryBuffer);
	RhiRayTraceGeometryBuffer(RHI_RT_BVH* handle = nullptr);
	virtual ~RhiRayTraceGeometryBuffer() = default;
	void create(const RhiDevice& device, const RhiCommandBuffer& command_buffer,
		const RhiGPUBuffer& vertex_buffer, const RhiGPUBuffer* index_buffer = nullptr);
	void create(const RhiDevice& device, const RhiCommandBuffer& command_buffer,
		const std::vector<RHI_BUFFER*>& vertex_buffers, const std::vector<RHI_BUFFER*>& index_buffers);
	void create(const RhiDevice& device, const RhiCommandBuffer& command_buffer,
		const RhiGPUBuffer& vertex_buffer, const size_t vertices_offset, const size_t vertices_length,
		const RhiGPUBuffer* index_buffer, const size_t indices_offset, const size_t indices_length);
private:
	RhiRayTraceGeometrydBufferInstances m_instances;
};
#endif // __rhi_ray_trace_geometry_buffer_hpp__