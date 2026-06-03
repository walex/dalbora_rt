#include "rhi_command_buffer.hpp"
#include "rhi_command_queue.hpp"
#include "rhi_device.hpp"
#include "rhi_gpu_buffer.hpp"

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