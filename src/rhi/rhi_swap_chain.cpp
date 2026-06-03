#include "rhi_swap_chain.hpp"
#include "rhi_command_queue.hpp"
#include "rhi_device.hpp"
#include "rhi_window.hpp"

RhiSwapChain::RhiSwapChain(RHI_SWAP_CHAIN* handle) : RhiImpl<RHI_SWAP_CHAIN>(handle) {}

void RhiSwapChain::create(const RhiWindow& window, const RhiDevice& device, 
	const RhiCommandQueue& command_queue, size_t buffers_count) {

	RHI_SWAP_CHAIN_DESC desc;
	desc.device = device;
	desc.command_queue = command_queue;
	desc.window = window;
	desc.width = window.get_width();
	desc.height = window.get_height();
	desc.allow_tearing = false;
	desc.buffer_count = buffers_count;
	desc.color_format = resource_format_R8G8B8A8_norm;
	this->set_handle(rhi_swap_chain_create(&desc));
}

RhiTextureView RhiSwapChain::get_next_render_target() {

	return RhiTextureView(const_cast<RHI_VIEW*>(rhi_swap_chain_get_surface(*this, UINT64_MAX)));
}

void RhiSwapChain::present() {
	rhi_swap_chain_present(*this);
}