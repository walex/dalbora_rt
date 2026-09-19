#include "rhi_buffer.hpp"
#include "rhi.hpp"

RhiView RhiBuffer::new_view(const RhiDevice& device, const shader_view_type shader_type, const RhiMemoryDescriptor* memory_descriptor_slot) {

	RhiView view;
	view.create(device, *this, memory_descriptor_slot, shader_type, this->get_format());
	return view;
}