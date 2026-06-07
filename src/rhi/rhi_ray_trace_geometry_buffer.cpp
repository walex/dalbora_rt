#include "rhi_ray_trace_geometry_buffer.hpp"
#include "rhi.hpp"

void RhiRayTraceGeometrydBufferInstances::create(const RhiDevice& device, const RhiCommandBuffer& command_buffer,
	RhiRayTraceGeometryBuffer& geometry_buffer, const std::vector<float*>& instance_transforms) {

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
	RhiRayTraceGeometryBuffer& geometry_buffer, const std::vector<float*>& instance_transforms) {

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

RhiRayTraceGeometryBuffer::RhiRayTraceGeometryBuffer(RHI_RT_BVH* handle)
	: RhiImpl<RHI_RT_BVH>(handle) {}

void RhiRayTraceGeometryBuffer::create_direct(const RhiDevice& device, const RhiCommandBuffer& command_buffer,
	const std::vector<std::vector<float*>>& transforms,
	const std::vector<RHI_BUFFER*>& vertex_buffers, const std::vector<RHI_BUFFER*>& index_buffers) {
	 
	RHI_RT_BVH_DESC desc;
	desc.device = device;
	desc.command_buffer = command_buffer;
	desc.vertex_buffer = &vertex_buffers[0];
	desc.index_buffer = &index_buffers[0];
	desc.count = vertex_buffers.size();
	this->set_handle(rhi_rt_bvh_create(&desc));
	for (size_t i = 0; i < vertex_buffers.size(); i++) {
		auto& instances = m_geomtric_instances.emplace_back();
		instances.create(device, command_buffer,
			*this, transforms[i]);
	}

}

void RhiRayTraceGeometryBuffer::create(const RhiDevice& device, const RhiCommandBuffer& command_buffer,
	const std::vector<float*>& instance_transforms,
	const RhiGPUBuffer& vertex_buffer, const RhiGPUBuffer* index_buffer) {

	RHI_RT_BVH_DESC desc;
	desc.device = device;
	desc.command_buffer = command_buffer;
	RHI_BUFFER* buffer = vertex_buffer;
	desc.vertex_buffer = &buffer;
	RHI_BUFFER* idx_buffer = nullptr;
	if (index_buffer)
		idx_buffer = *index_buffer;
	desc.index_buffer = &idx_buffer;
	desc.count = 1;
	this->set_handle(rhi_rt_bvh_create(&desc));
	auto& instance = m_geomtric_instances.emplace_back();
	instance.create(device, command_buffer,
			*this, instance_transforms);	
}

std::vector<RhiView> RhiRayTraceGeometryBuffer::new_view(RhiDevice& device) {

	std::vector<RhiView> views;
	views.reserve(m_geomtric_instances.size());
	for (auto& instance : m_geomtric_instances) {
		RHI_VIEW_DESC desc;
		desc.device = device;
		desc.buffer = instance;
		desc.type = resource_type_rt_bvh_buffer;
		desc.slot_id = device.next_read_only_buffer_slot_id();
		views.emplace_back(rhi_buffers_create_view(&desc));
	}
	return views;
}

void RhiRayTraceGeometryBuffer::update(const RhiDevice& device, const RhiCommandBuffer& command_buffer,
	const std::vector<float*>& instance_transforms) {

	for (auto& instance : m_geomtric_instances) {
		instance.update(device, command_buffer, *this, instance_transforms);
	}
}