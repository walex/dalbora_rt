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
				const RhiCommandQueue&, const size_t>
	, public RhiImpl<RHI_SWAP_CHAIN>{

public:
	RhiSwapChain(RHI_SWAP_CHAIN* hanlde = nullptr);
	virtual ~RhiSwapChain() = default;
	void create(const RhiWindow& window, const RhiDevice& device, const RhiCommandQueue& command_queue, const size_t buffers_count = 3) override;
	RhiView get_next_render_target();
	resource_format get_format();
	void blit(RhiCommandBuffer& command_buffer, RhiTexture& image);
	void present();
};

#endif // __rhi_swap_chain_hpp__