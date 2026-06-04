#ifndef __rhi_ray_trace_pipeline_hpp__
#define __rhi_ray_trace_pipeline_hpp__

#include "rhi_pipeline.hpp"

class RhiShaderProgram;
class RhiRayTracePipelineShaderPrograms {	
public:
	RhiShaderProgram* ray_gen_shader = nullptr;
	RhiShaderProgram* miss_shader = nullptr;
	RhiShaderProgram* closest_hit_shader = nullptr;
	std::vector<RHI_RT_SHADER_UNIT_DESC> ray_gen_shaders_desc;
	std::vector<RHI_RT_SHADER_UNIT_DESC> miss_shaders_desc;
	std::vector<RHI_RT_HIT_GROUP_DESC> hit_groups_desc;
};
class RhiDevice;
class RhiPipelineLayout;
class RhiRayTracePipeline
	: public RhiPipeline<RHI_RT_PIPELINE>
	, public ICreateRhiObject<const RhiDevice&, const RhiPipelineLayout&,
	const RhiRayTracePipelineShaderPrograms&> {
public:

	RhiRayTracePipeline(RHI_RT_PIPELINE* handle = nullptr);
	virtual ~RhiRayTracePipeline() = default;

	void create(const RhiDevice& device, const RhiPipelineLayout& layout,
		const RhiRayTracePipelineShaderPrograms& shader_programs);
private:
	std::vector<RHI_INPUT_LAYOUT_DESC> m_input_descriptors;
};

#endif // __rhi_ray_trace_pipeline_hpp__
