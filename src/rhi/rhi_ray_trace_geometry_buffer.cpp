#include "rhi_ray_trace_geometry_buffer.hpp"
#include "rhi_device.hpp"
#include "rhi_command_buffer.hpp"
#include "rhi_gpu_buffer.hpp"
#include "rhi_view.hpp"

void RhiRayTraceGeometrydBufferInstances::create(const RhiDevice& device, const RhiCommandBuffer& command_buffer,
	RhiRayTraceGeometrydBuffer& geometry_buffer, const std::vector<float*>& instance_transforms) {

	RHI_RT_BVH_GEOMETRY_INSTANCES_DESC tlas_desc;
	tlas_desc.device = device;
	tlas_desc.command_buffer = command_buffer;
	tlas_desc.parent_bvh = geometry_buffer;
	tlas_desc.transforms = const_cast<float**>(instance_transforms.data());
	tlas_desc.instance_count = instance_transforms.size();
	tlas_desc.read_only = false;
	this->set_handle(RhiRayTraceGeometrydBufferInstances(rhi_rt_bvh_build_geometry_instances(&tlas_desc)));
}

void RhiRayTraceGeometrydBufferInstances::update(const RhiDevice& device, const RhiCommandBuffer& command_buffer,
	RhiRayTraceGeometrydBuffer& geometry_buffer, const std::vector<float*>& instance_transforms) {

	RHI_RT_BVH_GEOMETRY_INSTANCES_DESC desc;
	desc.device = device;
	desc.command_buffer = command_buffer;
	desc.parent_bvh = geometry_buffer;
	desc.transforms = const_cast<float**>(instance_transforms.data());
	desc.instance_count = instance_transforms.size();;
	rhi_rt_bvh_update_geometry_instances(&desc, *this);
}

RhiRayTraceGeometrydBufferInstances::RhiRayTraceGeometrydBufferInstances(RHI_BUFFER* handle)
	: RhiImpl<RHI_BUFFER>(handle) {}

RhiRayTraceGeometrydBuffer::RhiRayTraceGeometrydBuffer(RHI_RT_BVH* handle)
	: RhiImpl<RHI_RT_BVH>(handle) {}

void RhiRayTraceGeometrydBuffer::create(const RhiDevice& device, const RhiCommandBuffer& command_buffer,
	const std::vector<float*>& instance_transforms,
	const RhiGPUBuffer& vertex_buffer, const RhiGPUBuffer* index_buffer) {

	RHI_RT_BVH_DESC desc;
	desc.device = device;
	desc.command_buffer = command_buffer;
	desc.vertex_buffer = vertex_buffer;
	RHI_BUFFER* idx_buffer = nullptr;
	if (index_buffer)
		idx_buffer = *index_buffer;
	desc.index_buffer = idx_buffer;
	this->set_handle(rhi_rt_bvh_create(&desc));

	m_geomtric_instances.create(device, command_buffer,
		*this, instance_transforms);	
}

RhiView RhiRayTraceGeometrydBuffer::new_view(RhiDevice& device) {

	RHI_VIEW_DESC desc;
	desc.device = device;
	desc.buffer = m_geomtric_instances;
	desc.type = resource_type_rt_bvh_buffer;
	desc.slot_id = device.next_read_only_buffer_slot_id();
	return RhiView(rhi_buffers_create_view(&desc));
}

void RhiRayTraceGeometrydBuffer::update(const RhiDevice& device, const RhiCommandBuffer& command_buffer,
	const std::vector<float*>& instance_transforms) {

	m_geomtric_instances.update(device, command_buffer, *this, instance_transforms);
}