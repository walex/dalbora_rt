#include "rhi_ray_trace_geometry_buffer.hpp"
#include "rhi.hpp"


void RhiRayTraceGeometrydBufferInstances::create(const RhiDevice& device, const RhiCommandBuffer& command_buffer,
	const RhiRayTraceGeometryBuffer& geometry_buffer,
	const std::vector<float*>& instance_transforms) {

	RHI_RT_BVH_GEOMETRY_DESC desc;
	desc.device = device;
	desc.command_buffer = command_buffer;
	desc.instance_info_count = 1;
	desc.read_only = false;
	RHI_RT_BVH_GEOMETRY_INSTANCE_DESC geometry_instances_desc;
	geometry_instances_desc.parent_bvh = geometry_buffer;
	geometry_instances_desc.transforms = instance_transforms.data();
	geometry_instances_desc.transforms_count = instance_transforms.size();
	desc.instance_info = &geometry_instances_desc;
	desc.total_instances = geometry_instances_desc.transforms_count;
	this->set_handle(rhi_rt_bvh_build_geometry_instances(&desc));
}

void RhiRayTraceGeometrydBufferInstances::create(const RhiDevice& device, const RhiCommandBuffer& command_buffer,
	const std::vector<RhiRayTraceGeometryBuffer>& geometry_buffer,
	const std::vector<std::vector<float*>>& instance_transforms) {

	RHI_RT_BVH_GEOMETRY_DESC desc;
	desc.device = device;
	desc.command_buffer = command_buffer;
	desc.instance_info_count = geometry_buffer.size();
	desc.read_only = false;
	std::vector<RHI_RT_BVH_GEOMETRY_INSTANCE_DESC> geometry_instances_desc(geometry_buffer.size());
	size_t total_instances = 0;
	for (size_t i = 0; i < geometry_buffer.size(); i++) {
		auto& idesc = geometry_instances_desc.at(i);
		idesc.parent_bvh = geometry_buffer[i];
		idesc.transforms = instance_transforms[i].data();
		idesc.transforms_count = instance_transforms[i].size();
		total_instances += idesc.transforms_count;
	}
	desc.instance_info = geometry_instances_desc.data();
	desc.total_instances = total_instances;
	this->set_handle(rhi_rt_bvh_build_geometry_instances(&desc));
}

void RhiRayTraceGeometrydBufferInstances::update(const RhiDevice& device, const RhiCommandBuffer& command_buffer,
	RhiRayTraceGeometryBuffer& geometry_buffer, const std::vector<float*>& instance_transforms) {
	
	RHI_RT_BVH_GEOMETRY_INSTANCE_DESC inst_desc;
	RHI_RT_BVH_GEOMETRY_DESC tlas_desc;
	tlas_desc.device = device;
	tlas_desc.command_buffer = command_buffer;
	tlas_desc.instance_info = &inst_desc;
	tlas_desc.instance_info_count = 1;
	inst_desc.parent_bvh = geometry_buffer;
	inst_desc.transforms = const_cast<float**>(instance_transforms.data());
	inst_desc.transforms_count = instance_transforms.size();
	tlas_desc.read_only = false;
	tlas_desc.total_instances = instance_transforms.size();
	rhi_rt_bvh_update_geometry_instances(&tlas_desc, *this);
}

RhiView RhiRayTraceGeometrydBufferInstances::new_view(RhiDevice& device) {

	RHI_VIEW_DESC desc;
	desc.device = device;
	desc.buffer = *this;
	desc.type = resource_type_rt_bvh_buffer;
	desc.slot_id = device.next_read_only_buffer_slot_id();
	return RhiView(rhi_buffers_create_view(&desc));
}

RhiRayTraceGeometrydBufferInstances::RhiRayTraceGeometrydBufferInstances(RHI_BUFFER* handle)
	: RhiImpl<RHI_BUFFER>(handle) {}

RhiRayTraceGeometryBuffer::RhiRayTraceGeometryBuffer(RHI_RT_BVH* handle)
	: RhiImpl<RHI_RT_BVH>(handle) {}

void RhiRayTraceGeometryBuffer::create(const RhiDevice& device, const RhiCommandBuffer& command_buffer,
	const std::vector<RHI_BUFFER*>& vertex_buffers, const std::vector<RHI_BUFFER*>& index_buffers) {
	 
	RHI_RT_BVH_DESC desc;
	desc.device = device;
	desc.command_buffer = command_buffer;
	desc.vertex_buffer = vertex_buffers.data();
	desc.index_buffer = index_buffers.data();
	desc.count = vertex_buffers.size();
	this->set_handle(rhi_rt_bvh_create(&desc));
}

void RhiRayTraceGeometryBuffer::create(const RhiDevice& device, const RhiCommandBuffer& command_buffer,
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
}

