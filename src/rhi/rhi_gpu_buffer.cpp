#include "rhi_gpu_buffer.hpp"
#include "rhi_device.hpp"
#include "rhi_command_buffer.hpp"
#include "rhi_shared_buffer.hpp"

RhiGPUBuffer::RhiGPUBuffer(RHI_BUFFER* handle)
	: RhiImpl<RHI_BUFFER>(handle) {}

void RhiGPUBuffer::create(const RhiDevice& device, const size_t length, const size_t stride, resource_format format) {

	RHI_BUFFER_DESC desc;
	desc.device = device;
	desc.length = length;
	desc.memory_type = buffer_memory_type_gpu_only;
	desc.type = buffer_type_raw;
	desc.format = format;
	desc.mips = 1;
	desc.stride = stride;
	this->set_handle(rhi_buffers_create_raw(&desc));
}

void RhiGPUBuffer::upload(const RhiCommandBuffer& command_buffer, const RhiSharedBuffer& sb) {

	rhi_buffers_gpu_upload(command_buffer, sb, *this);
}

resource_format RhiGPUBuffer::get_format() {
	return static_cast<RHI_BUFFER*>(*this)->format;
}

RhiView RhiGPUBuffer::new_depth_buffer_view(RhiDevice& device) {
	RHI_VIEW_DESC desc;
	desc.device = device;
	desc.buffer = *this;
	desc.type = resource_type_depth_stencil_target;
	desc.format = this->get_format();
	desc.slot_id = device.next_depth_buffer_slot_id();
	return RhiView(rhi_buffers_create_view(&desc));
}

RhiView RhiGPUBuffer::new_constant_buffer_view(RhiDevice& device) {

	RHI_VIEW_DESC object_cb_view_desc;
	object_cb_view_desc.device = device;
	object_cb_view_desc.buffer = *this;
	object_cb_view_desc.type = resource_type_constant_buffer;
	object_cb_view_desc.slot_id = device.next_constant_buffer_slot_id();
	return RhiView(rhi_buffers_create_view(&object_cb_view_desc));
}