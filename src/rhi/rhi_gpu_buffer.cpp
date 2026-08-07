#include "rhi_gpu_buffer.hpp"
#include "rhi.hpp"

RhiGPUBuffer::RhiGPUBuffer(RHI_BUFFER* handle, buffer_memory_type type)
	: RhiBuffer(handle) {

	// TODO: 
	// review buffer_memory_type enum
	// explicit actual state  D3D12_HEAP_TYPE_DEFAULT
	// Implement buffer_access_flags_rw with D3D12_HEAP_TYPE_DEFAULT + ALLOW_UNORDERED_ACCESS
}

void RhiGPUBuffer::create(const RhiDevice& device, const size_t length,
	const size_t stride, resource_format format) {

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

void RhiGPUBuffer::upload(const RhiCommandBuffer& command_buffer, const RhiSharedBuffer& sb,
	const size_t offset_src, const size_t offset_dest, const size_t length) {

	rhi_buffers_gpu_upload_region(command_buffer, sb, *this, offset_src, offset_dest, length);
		
}

resource_format RhiGPUBuffer::get_format() { return static_cast<RHI_BUFFER*>(*this)->format; }
size_t RhiGPUBuffer::get_length() { return static_cast<RHI_BUFFER*>(*this)->length; }
size_t RhiGPUBuffer::get_stride() { return static_cast<RHI_BUFFER*>(*this)->stride; }

RhiView RhiGPUBuffer::new_depth_buffer_view(const RhiDevice& device) {
	RHI_VIEW_DESC desc;
	desc.device = device;
	desc.buffer = *this;
	desc.type = resource_type_depth_stencil_target;
	desc.format = this->get_format();
	desc.slot_id = device.next_depth_buffer_slot_id();
	return RhiView(rhi_buffers_create_view(&desc), desc.slot_id);
}

RhiView RhiGPUBuffer::new_constant_buffer_view(const RhiDevice& device) {

	RHI_VIEW_DESC object_cb_view_desc;
	object_cb_view_desc.device = device;
	object_cb_view_desc.buffer = *this;
	object_cb_view_desc.type = resource_type_constant_buffer;
	object_cb_view_desc.slot_id = device.next_constant_buffer_slot_id();
	return RhiView(rhi_buffers_create_view(&object_cb_view_desc), object_cb_view_desc.slot_id);
}

RhiView RhiGPUBuffer::new_shader_view(const RhiDevice& device) {
	RHI_VIEW_DESC desc;
	desc.device = device;
	desc.buffer = *this;
	desc.type = resource_type_shader;
	desc.format = this->get_format();
	desc.slot_id = device.next_rw_buffer_slot_id();
	return RhiView(rhi_buffers_create_view(&desc), desc.slot_id);
}