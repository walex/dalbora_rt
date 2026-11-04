#ifndef __rhi_raster_pipeline_hpp__
#define __rhi_raster_pipeline_hpp__

#include "rhi_pipeline.hpp"

class RhiShaderProgramHLSL;
class RhiRasterPipelineShaderPrograms {	
public:
	RhiShaderProgramHLSL* vertex_shader = nullptr;
	RhiShaderProgramHLSL* pixel_shader = nullptr;
	RhiShaderProgramHLSL* geometry_shader = nullptr;
	RhiShaderProgramHLSL* hull_shader = nullptr;
	RhiShaderProgramHLSL* tess_shader = nullptr;
	RhiShaderProgramHLSL* domain_shader = nullptr;
	RhiShaderProgramHLSL* as_shader = nullptr;
	RhiShaderProgramHLSL* mesh_shader = nullptr;
};
class RhiDevice;
class RhiPipelineLayout;
class RhiRasterPipeline
	: public RhiPipeline<RHI_RASTER_PIPELINE>
	, public ICreateRhiObject<const RhiDevice&, const RhiPipelineLayout&,
	const RhiRasterPipelineShaderPrograms&> {
public:

	RhiRasterPipeline(RHI_RASTER_PIPELINE* handle = nullptr);
	virtual ~RhiRasterPipeline() = default;

	void create(const RhiDevice& device, const RhiPipelineLayout& layout,
		const RhiRasterPipelineShaderPrograms& shader_programs);
	std::vector<RHI_INPUT_LAYOUT_DESC>& get_input_descriptors() { return m_input_descriptors; }
	void add_input_descriptor(const std::string& name, size_t offset, resource_format format);
private:
	std::vector<RHI_INPUT_LAYOUT_DESC> m_input_descriptors;
};

#endif // __rhi_raster_pipeline_hpp__
