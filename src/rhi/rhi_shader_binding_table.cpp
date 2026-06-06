#include "rhi_shader_binding_table.hpp"
#include "rhi.hpp"

RhiShaderBindingTable::RhiShaderBindingTable(RHI_SBT_TABLE* handle)
	: RhiImpl<RHI_SBT_TABLE>(handle) {}

void RhiShaderBindingTable::create(const RhiDevice& device, 
	const RhiRayTracePipeline& pipeline,
	const RhiRayTracePipelineShaderPrograms& shader_programs) {

	RHI_RT_SBT_DESC sbt_desc;
	for (size_t i = 0; i < shader_programs.ray_gen_shaders_desc.size(); i++) {
		sbt_desc.ray_gen_ids[i] = shader_programs.ray_gen_shaders_desc.at(i).name_id;
	}	
	sbt_desc.ray_gen_count = shader_programs.ray_gen_shaders_desc.size();
	for (size_t i = 0; i < shader_programs.miss_shaders_desc.size(); i++) {
		sbt_desc.miss_ids[i] = shader_programs.miss_shaders_desc.at(i).name_id;
	}
	sbt_desc.miss_shader_count = shader_programs.ray_gen_shaders_desc.size();
	for (size_t i = 0; i < shader_programs.hit_groups_desc.size(); i++) {
		sbt_desc.hit_group_ids[i] = shader_programs.hit_groups_desc.at(i).name_id;
	}
	sbt_desc.hit_group_count = shader_programs.hit_groups_desc.size();
	this->set_handle(rhi_rt_pipeline_create_sbt(device, &sbt_desc, pipeline));
}

