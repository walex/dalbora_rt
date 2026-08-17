#include "RayRadianceIntegratorGPU.hpp"
#include "Culler.hpp"
#include "RayTracingRenderer.hpp"
#include "SurfaceRadiance.hpp"
#include "Camera.hpp"
#include "Rays.hpp"

RayRadianceIntegratorGPU::RayRadianceIntegratorGPU(
	const RhiDevice& device, RhiCommandQueue& command_queue,
	const RhiShaderProgram& ray_gen_shader, const RhiShaderProgram& miss_shader,
	const RhiShaderProgram& closest_hit_shader, resource_format surface_format) {

	// save shader programs references

	m_ray_gen_shader = &ray_gen_shader;
	m_miss_shader = &miss_shader;
	m_closest_hit_shader = &closest_hit_shader;
	m_command_queue = &command_queue;
	m_command_buffer.create(device, command_queue);

	// config ray trace shader
	std::string ray_gen_entry_point = "RayGen";
	std::string miss_entry_point = "Miss";
	std::string closest_hit_entry_point = "ClosestHit";

	RhiRayTracePipelineShaderPrograms ray_trace_shader_programs;
	std::vector<RHI_RT_HIT_GROUP_DESC>& hit_groups_desc = ray_trace_shader_programs.hit_groups_desc;
	auto& hg = hit_groups_desc.emplace_back();
	strcpy_s(hg.name_id, "HG_1");
	hg.closest_hit.blob = closest_hit_shader;
	strcpy_s(hg.closest_hit.name_id, closest_hit_entry_point.c_str());

	std::vector<RHI_RT_SHADER_UNIT_DESC>& miss_shader_desc = ray_trace_shader_programs.miss_shaders_desc;
	auto& miss_1 = miss_shader_desc.emplace_back();
	miss_1.blob = miss_shader;
	strcpy_s(miss_1.name_id, miss_entry_point.c_str());

	std::vector<RHI_RT_SHADER_UNIT_DESC>& ray_gen_shader_desc = ray_trace_shader_programs.ray_gen_shaders_desc;
	auto& ray_gen = ray_gen_shader_desc.emplace_back();
	strcpy_s(ray_gen.name_id, ray_gen_entry_point.c_str());
	ray_gen.blob = ray_gen_shader;

	ray_trace_shader_programs.ray_gen_shader = &ray_gen_shader;
	ray_trace_shader_programs.miss_shader = &miss_shader;
	ray_trace_shader_programs.closest_hit_shader = &closest_hit_shader;

	// create pipeline layout
	m_pipeline_layout.create(device, primitive_topology_triangle, surface_format, resource_format_d24_norm_s8_uint);

	// rt pipeline config
	m_pipeline.create(device, m_pipeline_layout, ray_trace_shader_programs);

	// shader binding table
	m_sbt.create(device, m_pipeline, ray_trace_shader_programs);
}

void RayRadianceIntegratorGPU::run(
	const Camera& camera,
	const std::vector<Mesh*>& geometries,
	const std::vector<Mesh*>& lights,
	const std::vector<SurfaceRadiance*>& out_radiances) {

	m_command_queue->sync_exec([&](RhiCommandQueueBufferList& command_buffers_list) {
		MosaicIntegrator::run(camera, geometries, lights, out_radiances);
		command_buffers_list.add_command_buffer(m_command_buffer);
	});
}

std::shared_ptr<MosaicIntegratorTaskParams> RayRadianceIntegratorGPU::create_params() {

	std::shared_ptr<RayRadianceIntegratorGPUTaskParams> params = std::make_shared<RayRadianceIntegratorGPUTaskParams>();
	return params;
}

Rays RayRadianceIntegratorGPU::generateRays(const Camera& UNUSED_PARAM(camera), const Samples& UNUSED_PARAM(samples)) {

	// rays are generated in a shader so do nothing
	static Rays rays = Rays();
	return rays;
}

void RayRadianceIntegratorGPU::LI(const Rays& UNUSED_PARAM(rays), const std::vector<Mesh*>& UNUSED_PARAM(geometries),
	const std::vector<Mesh*>& UNUSED_PARAM(lights), const size_t tile[4], SurfaceRadiance& out_radiance) {
	
	//m_command_buffer.record([&] {
	//	m_command_buffer.ray_trace(static_cast<SurfaceRadianceGPU&>(out_radiance), m_sbt);
	//});
}