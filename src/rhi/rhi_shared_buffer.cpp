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

resource_format RhiSharedBuffer::get_format() { return static_cast<RHI_BUFFER*>(*this)->format; }

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
		*this,
		offset,
		length);
}

void RhiSharedBuffer::unmap(const RhiSharedBufferMap& map_info) {

	rhi_buffers_map_close(*this, map_info.get_offset(), map_info.get_length());
}

void RhiSharedBuffer::copy(const uint8_t* data, const size_t length, const size_t offset) {

	auto v_map_info = this->map(offset, length);
	memcpy(v_map_info.get_data() + offset, data, length);
}

RhiView RhiSharedBuffer::new_depth_buffer_view(const RhiDevice& UNUSED_PARAM(device)) {

	throw std::exception("mappeable depth bufferview is not supported");
}

RhiView RhiSharedBuffer::new_constant_buffer_view(const RhiDevice& device) {

	RHI_VIEW_DESC desc;
	desc.device = device;
	desc.buffer = *this;
	desc.type = resource_type_constant_buffer;
	desc.slot_id = device.next_constant_buffer_slot_id();
	return RhiView(rhi_buffers_create_view(&desc), static_cast<int>(desc.slot_id));
}


RhiView RhiSharedBuffer::new_shader_read_only_view(const RhiDevice& device) {

	RHI_VIEW_DESC desc;
	desc.device = device;
	desc.buffer = *this;
	desc.type = resource_type_read_only_shader_buffer;
	desc.format = this->get_format();
	desc.slot_id = device.next_read_only_buffer_slot_id();
	return RhiView(rhi_buffers_create_view(&desc), static_cast<int>(desc.slot_id));
}