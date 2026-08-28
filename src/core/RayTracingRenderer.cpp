#include "RayTracingRenderer.hpp"
#include "SurfaceRadiance.hpp"

RayTracingRenderer::RayTracingRenderer(const resource_format surface_format, const size_t surface_width,
	const size_t surface_height, const size_t read_only_shader_registers_count,
	const size_t rw_shader_registers_count, const size_t constant_shader_registers_count) {
	
	RHI_DEVICE_DESC device_desc;
	device_desc.features |= device_features_raytracing;
	device_desc.shader_model = hlsl_shader_model_6_8;
	device_desc.shader_resources_desc.read_only_buffer_shader_registers_count = read_only_shader_registers_count;
	device_desc.shader_resources_desc.rw_buffer_shader_registers_count = rw_shader_registers_count;
	device_desc.shader_resources_desc.constant_buffer_shader_registers_count = constant_shader_registers_count;

	this->create(device_desc);

	m_ray_trace_surface.create(this->get_device(), surface_format,
		surface_width, surface_height);
	m_ray_trace_surface_view = m_ray_trace_surface.new_rw_view(this->get_device());
	m_ray_trace_render_pass.create(this->get_device());
}

void RayTracingRenderer::on_draw(RhiView& out_surface_view)
{
	ASSERT_PTR(m_sbt);

	m_ray_trace_render_pass.set_render_target(m_ray_trace_surface_view);

	m_ray_trace_render_pass.render(this->get_command_buffer(), [&](RhiCommandBuffer& command_buffer) {

		command_buffer.ray_trace(m_ray_trace_surface, *m_sbt);
	});

	out_surface_view.blit(this->get_command_buffer(), m_ray_trace_surface);
}