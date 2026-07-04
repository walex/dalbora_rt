#include "rhi_swap_chain.hpp"
#include "rhi.hpp"

RhiSwapChain::RhiSwapChain(RHI_SWAP_CHAIN* handle) : RhiImpl<RHI_SWAP_CHAIN>(handle) {}

void RhiSwapChain::create(const RhiWindow& window, const RhiDevice& device, 
	const RhiCommandQueue& command_queue, size_t buffers_count, bool enable_vertical_sync) {

	RHI_SWAP_CHAIN_DESC desc;
	desc.device = device;
	desc.command_queue = command_queue;
	desc.window = window;
	desc.width = window.get_width();
	desc.height = window.get_height();
	desc.disable_vsync = !enable_vertical_sync;
	desc.buffer_count = buffers_count;
	desc.color_format = resource_format_R8G8B8A8_norm;
	this->set_handle(rhi_swap_chain_create(&desc));
}

RhiView RhiSwapChain::get_next_render_target() {

	return RhiView(const_cast<RHI_VIEW*>(rhi_swap_chain_get_surface(*this, UINT64_MAX)));
}

resource_format RhiSwapChain::get_format() {
	return static_cast<RHI_SWAP_CHAIN*>(*this)->format;
}

void RhiSwapChain::blit(RhiCommandBuffer& command_buffer, RhiTexture& image) {
	const RHI_VIEW* back_buffer = rhi_swap_chain_get_surface(*this, INT64_MAX);
	rhi_command_buffer_copy_texture(command_buffer, dynamic_cast<RHI_TEXTURE_2D*>(back_buffer->buffer), image);
}

void RhiSwapChain::present() {
	rhi_swap_chain_present(*this);
}

