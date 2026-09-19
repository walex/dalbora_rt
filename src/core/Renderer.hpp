#ifndef __Renderer_h__
#define __Renderer_h__

#include "Common.hpp"

class ResourceManager;
class Renderer {
public:
	Renderer(ResourceManager& rm) : m_resource_manager(rm) {};
	virtual ~Renderer() = default;
	void create();
	virtual void render(RhiView& out_surface, const RHI_VIEWPORT& view_port);
	RhiSwapChain create_swap_chain(const RhiWindow& window, const uint32_t buffer_count,
		const resource_format surface_format, const bool enable_vertical_sync);
	RhiGraphicsCommandQueue& get_command_queue() { return m_command_queue; }
	virtual void set_rt_pipeline(RhiRayTracePipeline& pipeline) {}
	virtual void set_bindig_table(RhiShaderBindingTable& sbt) {}

	RhiCommandBuffer& get_command_buffer() { return m_command_buffer; }
protected:
	virtual void on_render(RhiView& surface) = 0;

	ResourceManager& m_resource_manager;
private:
	RhiGraphicsCommandQueue m_command_queue;
	RhiCommandBuffer m_command_buffer;
	RhiRasterRenderPass m_raster_render_pass;
};

#endif