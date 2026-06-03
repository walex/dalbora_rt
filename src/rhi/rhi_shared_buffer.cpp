#include "rhi_shared_buffer.hpp"
#include "rhi_device.hpp"

RhiSharedBuffer::RhiSharedBuffer(RHI_BUFFER* handle)
	: RhiImpl<RHI_BUFFER>(handle) {}

void RhiSharedBuffer::create(const RhiDevice& device, const size_t length, const size_t stride) {

	RHI_BUFFER_DESC desc;
	desc.device = device;
	desc.length = length;
	desc.memory_type = buffer_memory_type_shared_rw;
	desc.type = buffer_type_raw;
	desc.format = resource_format_none;
	desc.stride = stride;
	desc.mips = 1;
	this->set_handle(rhi_buffers_create_raw(&desc));
}

RhiSharedBufferMap RhiSharedBuffer::map(const size_t offset, const size_t length) {

	return RhiSharedBufferMap(
		offset,
		length,
		static_cast<uint8_t*>(rhi_buffers_map_open(*this, offset, length)));
}

void RhiSharedBuffer::unmap(const RhiSharedBufferMap& map_info) {

	rhi_buffers_map_close(*this, map_info.get_offset(), map_info.get_length());
}

RhiView RhiSharedBuffer::new_depth_buffer_view(RhiDevice& device) {

	throw std::exception("mappeable depth bufferview is not supported");
}

RhiView RhiSharedBuffer::new_constant_buffer_view(RhiDevice& device) {

	RHI_VIEW_DESC object_cb_view_desc;
	object_cb_view_desc.device = device;
	object_cb_view_desc.buffer = *this;
	object_cb_view_desc.type = resource_type_constant_buffer;
	object_cb_view_desc.slot_id = device.next_constant_buffer_slot_id();
	return RhiView(rhi_buffers_create_view(&object_cb_view_desc));
}