#include "rhi_view.hpp"
#include "rhi.hpp"

RhiView::RhiView(RHI_VIEW* handle, bool ownership) 
	: RhiImpl<RHI_VIEW>(handle, ownership) {
	
	if (handle != nullptr && ownership == true) {
		
		ASSERT_PTR(handle->memory_descriptor);
		m_memory_descriptor.reset(handle->memory_descriptor);
	}
}

void RhiView::create(const RhiDevice& device, const RhiBuffer& buffer,
	RHI_MEMORY_DESCRIPTOR_SLOT* memory_descriptor_slot, const shader_view_type type,
	const resource_format format, const size_t mip_maps_count) {
	
	RHI_VIEW_DESC desc;
	desc.device = device;
	desc.buffer = buffer;
	desc.type = type;
	desc.format = format;
	desc.mip_maps_count = mip_maps_count;
	desc.memory_descriptor = memory_descriptor_slot;
	this->set_handle(rhi_buffers_create_view(&desc));
	m_memory_descriptor.reset(memory_descriptor_slot);
}

void RhiView::blit(RhiCommandBuffer& command_buffer, RhiTexture& image) {

	rhi_command_buffer_copy_texture(command_buffer, dynamic_cast<RHI_TEXTURE_2D*>(static_cast<const RHI_VIEW*>(*this)->buffer), image);
}