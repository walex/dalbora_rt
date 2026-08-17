#ifndef __Renderer_h__
#define __Renderer_h__

#include "Common.hpp"

class Renderer {
public:
	Renderer() = default;
	virtual ~Renderer() = default;
	void create(RHI_DEVICE_DESC& device_desc);
	virtual void draw(RhiView& surface, const RHI_VIEWPORT& view_port);
	RhiSwapChain create_swap_chain(const RhiWindow& window, const uint32_t buffer_count);
protected:
	virtual void on_draw(RhiView& surface) = 0;
	RhiDevice& get_device() { return m_device; }
	RhiGraphicsCommandQueue& get_command_queue() { return m_command_queue; }
	RhiCommandBuffer& get_command_buffer() { return m_command_buffer; }
private:
	RhiDevice m_device;
	RhiGraphicsCommandQueue m_command_queue;
	RhiCommandBuffer m_command_buffer;
	RhiRasterRenderPass m_render_pass;

};

#endif