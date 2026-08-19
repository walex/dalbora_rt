#ifndef __rhi_swap_chain_hpp__
#define __rhi_swap_chain_hpp__

#include "rhi_impl.hpp"
#include "rhi_view.hpp"

class RhiWindow;
class RhiDevice;
class RhiCommandQueue;
class RhiCommandBuffer;
class RhiTexture;
class RhiSwapChain 
	: public ICreateRhiObject<const RhiWindow&, const RhiDevice&, 
				const RhiCommandQueue&, const size_t, resource_format, const bool>
	, public RhiImpl<RHI_SWAP_CHAIN>{

public:
	RhiSwapChain(RHI_SWAP_CHAIN* hanlde = nullptr);
	RhiSwapChain(RhiSwapChain&&) noexcept = default;
	RhiSwapChain& operator=(RhiSwapChain&&) noexcept = default;
	RhiSwapChain(const RhiSwapChain&) = delete;
	RhiSwapChain& operator=(const RhiSwapChain&) = delete;
	virtual ~RhiSwapChain() = default;
	void create(const RhiWindow& window, const RhiDevice& device, const RhiCommandQueue& command_queue,
		const size_t buffers_count = 3,
		resource_format format = resource_format_R8G8B8A8_norm,
		bool enable_vertical_sync = false) override;
	RhiView get_next_render_target();
	resource_format get_format();
	void blit(RhiCommandBuffer& command_buffer, RhiTexture& image);
	void present();
};

#endif // __rhi_swap_chain_hpp__