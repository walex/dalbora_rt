#include "Renderer.hpp"
#include "ResourceManager.hpp"

void Renderer::create() {

	m_command_queue.create(m_resource_manager.get_device());
	m_command_buffer.create(m_resource_manager.get_device(), m_command_queue);
	m_command_buffer.set_buffers_memory_descriptor(m_resource_manager);
	m_raster_render_pass.create(m_resource_manager.get_device());
}

RhiSwapChain Renderer::create_swap_chain(const RhiWindow& window, const uint32_t buffer_count,
	resource_format surface_format) {
	
	RhiSwapChain swap_chain;
	swap_chain.create(window, m_resource_manager.get_device(), m_command_queue, buffer_count, surface_format, true);
	return swap_chain;
}

void Renderer::draw(RhiView& out_surface, const RHI_VIEWPORT& view_port) {

	m_command_buffer.record([&] {

		m_raster_render_pass.set_view_port(view_port);
		m_raster_render_pass.set_render_target(out_surface);
		m_raster_render_pass.render(m_command_buffer, [&](RhiCommandBuffer& UNUSED_PARAM(command_buffer)) {
			
			this->on_draw(out_surface);
		});
	});
	
	m_command_queue.sync_exec([&](RhiCommandQueueBufferList& list) {

		list.add_command_buffer(m_command_buffer);
	});
}