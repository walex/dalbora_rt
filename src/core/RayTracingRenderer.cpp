#include "RayTracingRenderer.hpp"
#include "SurfaceRadiance.hpp"

RayTracingRenderer::RayTracingRenderer() {
	
	constexpr size_t read_only_shader_registers_count = 800;
	constexpr size_t rw_shader_registers_count = 100;
	constexpr size_t constant_shader_registers_count = 1;

	RHI_DEVICE_DESC device_desc;
	device_desc.features |= device_features_raytracing;
	device_desc.shader_model = hlsl_shader_model_6_8;
	device_desc.shader_resources_desc.read_only_buffer_shader_registers_count = read_only_shader_registers_count;
	device_desc.shader_resources_desc.rw_buffer_shader_registers_count = rw_shader_registers_count;
	device_desc.shader_resources_desc.constant_buffer_shader_registers_count = constant_shader_registers_count;

	this->create(device_desc);
}

void RayTracingRenderer::on_draw(RhiView& surface)
{
	
	// compose and draw the surface radiance
	//for (auto& radiance : radiances) {

		//RhiGPUBuffer gpu_input_buffer = radiance;
		//render_target.upload_to_region(gpu_input_buffer, radiance.get_region());
	//}
}