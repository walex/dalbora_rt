#include "rhi_command_buffer.hpp"
#include "rhi_command_queue.hpp"
#include "rhi_device.hpp"
#include "rhi_gpu_buffer.hpp"
#include "rhi_render_target.hpp"
#include "rhi_ray_trace_geometry_buffer.hpp"
#include "rhi_shader_binding_table.hpp"

RhiCommandBuffer::RhiCommandBuffer(RHI_COMMAND_BUFFER* handle) : RhiImpl<RHI_COMMAND_BUFFER>(handle) {
}
void RhiCommandBuffer::create(const RhiDevice& device, const RhiCommandQueue& command_queue) {
	RHI_COMMAND_BUFFER_DESC command_buffer_desc;
	command_buffer_desc.device = device;
	command_buffer_desc.command_queue = command_queue;
	this->set_handle(rhi_command_buffer_create_for_render(&command_buffer_desc));
}

void RhiCommandBuffer::record(RhiCommandBufferRecordCallback callback) {
	
	rhi_command_buffer_record(*this,
		[&](RHI_VOID_PTR UNUSED_PARAM(native_command_buffer_impl)) {
			
			callback();			
	});
}

void RhiCommandBuffer::draw_triangle_list(RhiGPUBuffer& vertex_buffer, RhiGPUBuffer* index_buffer) {

	if (index_buffer)
		rhi_command_buffer_draw_triangle_list(*this, vertex_buffer, *index_buffer);
	else
		rhi_command_buffer_draw_triangle_list(*this, vertex_buffer, nullptr);
}


void RhiCommandBuffer::ray_trace(RhiRenderTarget& render_target, RhiRayTraceGeometrydBuffer& geometry_buffer, RhiShaderBindingTable& sbt) {

	rhi_command_buffer_ray_trace(*this, render_target, reinterpret_cast<RHI_BUFFER*>(static_cast<RHI_RT_BVH*>(geometry_buffer)), sbt);

};