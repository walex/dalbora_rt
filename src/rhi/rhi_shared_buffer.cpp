#include "rhi_shared_buffer.hpp"
#include "rhi.hpp"

RhiSharedBuffer::RhiSharedBuffer(RHI_BUFFER* handle, buffer_memory_type type)
	: RhiBuffer(handle) {

	// TODO:
	// review buffer_memory_type enum
	//  explicit actual state  D3D12_HEAP_TYPE_UPLOAD
	//  implement buffer_access_flags_read with D3D12_HEAP_TYPE_READBACK
	// implement buffer_access_flags_rw with D3D12_HEAP_TYPE_CUSTOM
}

void RhiSharedBuffer::create(const RhiDevice& device, const size_t length, 
	const size_t stride, const resource_format format) {

	RHI_BUFFER_DESC desc;
	desc.device = device;
	desc.length = length;
	desc.memory_type = buffer_memory_type_shared_rw;
	desc.type = buffer_type_raw;
	desc.format = format;
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

void RhiSharedBuffer::copy(const uint8_t* data, const size_t length, const size_t offset) {

	auto v_map_info = this->map(offset, length);
	memcpy(v_map_info.get_data() + offset, data, length);
	this->unmap(v_map_info);
}

RhiView RhiSharedBuffer::new_depth_buffer_view(RhiDevice& UNUSED_PARAM(device)) {

	throw std::exception("mappeable depth bufferview is not supported");
}

RhiView RhiSharedBuffer::new_constant_buffer_view(RhiDevice& device) {

	RHI_VIEW_DESC object_cb_view_desc;
	object_cb_view_desc.device = device;
	object_cb_view_desc.buffer = *this;
	object_cb_view_desc.type = resource_type_constant_buffer;
	object_cb_view_desc.slot_id = device.next_constant_buffer_slot_id();
	return RhiView(rhi_buffers_create_view(&object_cb_view_desc), static_cast<int>(object_cb_view_desc.slot_id));
}