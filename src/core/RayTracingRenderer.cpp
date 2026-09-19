#include "RayTracingRenderer.hpp"
#include "SurfaceRadiance.hpp"
#include "ResourceManager.hpp"

RayTracingRenderer::RayTracingRenderer(ResourceManager& rm, const resource_format surface_format, 
	const size_t surface_width, const size_t surface_height)
: Renderer(rm) {
	
	this->create();

	const RhiDevice& device = rm.get_device();
	m_ray_trace_surface.create(device, surface_format,
		surface_width, surface_height);
	m_ray_trace_surface_view = m_ray_trace_surface.new_view(device, m_resource_manager.get_rw_buffer_descriptor_slot());
	m_ray_trace_render_pass.create(device);
}
 
void RayTracingRenderer::on_render(RhiView& out_surface_view)
{
	ASSERT_PTR(m_sbt);

	m_ray_trace_render_pass.set_render_target(m_ray_trace_surface_view);
	m_ray_trace_render_pass.render(this->get_command_buffer(), [&](RhiCommandBuffer& command_buffer) {

		command_buffer.ray_trace(m_ray_trace_surface, *m_sbt);
	});
	out_surface_view.blit(this->get_command_buffer(), m_ray_trace_surface);
}