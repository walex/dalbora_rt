#include "rhi_buffer.hpp"
#include "rhi.hpp"

RhiView RhiBuffer::new_view(const RhiDevice& device, const RhiMemoryTable& descriptor_table,
	const shader_view_type shader_type) {
	//RHI_VIEW_DESC desc;
	//desc.device = device;
	//desc.buffer = *this;
	//desc.type = shader_type;
	//desc.format = this->get_format();
	//desc.memory_descriptor = descriptor_table.next_descriptor().release();
	RhiView view;
	view.create(device, *this, descriptor_table, shader_type, this->get_format());
	return view;
}