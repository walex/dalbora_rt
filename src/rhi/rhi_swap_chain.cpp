#include "rhi_swap_chain.hpp"
#include "rhi.hpp"

RhiSwapChain::RhiSwapChain(RHI_SWAP_CHAIN* handle) : RhiImpl<RHI_SWAP_CHAIN>(handle) {}

void RhiSwapChain::create(const RhiWindow& window, const RhiDevice& device, 
	const RhiCommandQueue& command_queue, size_t buffers_count, 
	resource_format format, bool enable_vertical_sync) {

	RHI_SWAP_CHAIN_DESC desc;
	desc.device = device;
	desc.command_queue = command_queue;
	desc.window = window;
	desc.width = window.get_width();
	desc.height = window.get_height();
	desc.disable_vsync = !enable_vertical_sync;
	desc.buffer_count = buffers_count;
	desc.color_format = format;
	this->set_handle(rhi_swap_chain_create(&desc));
	m_views.reserve(buffers_count);
}

void RhiSwapChain::create_views(RhiDevice& device, RhiMemoryTable& memory_descriptor) {

	size_t count = static_cast<RHI_SWAP_CHAIN*>(*this)->buffers_count;
	for (size_t buffer_id = 0; buffer_id < count; buffer_id++) {
		RHI_VIEW* view_ptr = rhi_swap_chain_create_view(device, *this, memory_descriptor.next_descriptor_ptr(), this->get_format(), buffer_id);
		m_views.emplace_back(view_ptr, true);
	}
}

RhiView RhiSwapChain::get_next_render_target() {

	size_t id = rhi_swap_chain_get_current_buffer_id(*this);
	return RhiView(static_cast<RHI_VIEW*>(m_views[id]));
}

resource_format RhiSwapChain::get_format() {
	return static_cast<RHI_SWAP_CHAIN*>(*this)->format;
}

void RhiSwapChain::blit(RhiCommandBuffer& command_buffer, RhiTexture& image) {
	
	size_t id = rhi_swap_chain_get_current_buffer_id(*this);
	m_views[id].blit(command_buffer, image);
}

void RhiSwapChain::present() {
	rhi_swap_chain_present(*this);
}

