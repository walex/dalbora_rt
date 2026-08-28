#include "rhi_view.hpp"
#include "rhi.hpp"

RhiView::RhiView(RHI_VIEW* handle, RhiDescriptorHeapResource&& slot)
	: RhiImpl<RHI_VIEW>(handle)
	, m_slot(std::move(slot)) {


}

RhiView::RhiView(RHI_VIEW* handle, int resource_id) 
	: RhiImpl<RHI_VIEW>(handle)
	, m_view_id(resource_id) {}

void RhiView::blit(RhiCommandBuffer& command_buffer, RhiTexture& image) {

	//const RHI_VIEW* back_buffer = *this;
	rhi_command_buffer_copy_texture(command_buffer, dynamic_cast<RHI_TEXTURE_2D*>(static_cast<const RHI_VIEW*>(*this)->buffer), image);
}