#include "rhi_command_buffer.hpp"
#include "rhi_command_queue.hpp"
#include "rhi_device.hpp"

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