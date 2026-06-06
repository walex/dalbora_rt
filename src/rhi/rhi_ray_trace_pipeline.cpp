#include "rhi_ray_trace_pipeline.hpp"
#include "rhi.hpp"

RhiRayTracePipeline::RhiRayTracePipeline(RHI_RT_PIPELINE* handle)
: RhiPipeline<RHI_RT_PIPELINE>(handle){}

void RhiRayTracePipeline::create(const RhiDevice& device, const RhiPipelineLayout& layout,
	const RhiRayTracePipelineShaderPrograms& shader_programs) {
		
	RHI_RT_PIPELINE_DESC desc;
	desc.device = device;
	desc.layout = layout;
	memcpy(desc.ray_gen, shader_programs.ray_gen_shaders_desc.data(), 
		sizeof(RHI_RT_SHADER_UNIT_DESC) * shader_programs.ray_gen_shaders_desc.size());
	desc.ray_gen_count = shader_programs.ray_gen_shaders_desc.size();
	memcpy(desc.miss_shaders, shader_programs.miss_shaders_desc.data(),
		sizeof(RHI_RT_SHADER_UNIT_DESC) * shader_programs.miss_shaders_desc.size());
	desc.miss_shader_count = shader_programs.miss_shaders_desc.size();
	memcpy(desc.hit_groups, shader_programs.hit_groups_desc.data(),
		sizeof(RHI_RT_HIT_GROUP_DESC) * shader_programs.hit_groups_desc.size());
	desc.hit_group_count = shader_programs.hit_groups_desc.size();
	this->set_handle(rhi_rt_pipeline_create(&desc));
}