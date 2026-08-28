
#ifndef __RayTracingRenderer_h__
#define __RayTracingRenderer_h__

#include "Renderer.hpp"

class RhiShaderBindingTable;
class RayTracingRenderer: public Renderer
{
public:
	RayTracingRenderer(const resource_format surface_format, const size_t surface_width,
		const size_t surface_height, const size_t read_only_shader_registers_count,
		const size_t rw_shader_registers_count, const size_t constant_shader_registers_count);
	virtual ~RayTracingRenderer() = default;
	void set_rt_pipeline(RhiRayTracePipeline& pipeline) override {
		m_ray_trace_render_pass.set_pipeline(pipeline);
	}
	void set_bindig_table(RhiShaderBindingTable& sbt) override {
		m_sbt = &sbt;
	}
protected:
	void on_draw(RhiView& out_surface_view);
private:
	RhiRenderTarget m_ray_trace_surface;
	RhiView m_ray_trace_surface_view;
	RhiRayTraceRenderPass m_ray_trace_render_pass;
	RhiShaderBindingTable *m_sbt = nullptr;
};

#endif // __RayTracingRenderer_h__
