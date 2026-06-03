#ifndef __rhi_raster_pipeline_hpp__
#define __rhi_raster_pipeline_hpp__

#include "rhi_impl.hpp"

class RhiShaderProgram;
class RhiPipelineShaderPrograms {	
public:
	RhiShaderProgram* vertex_shader = nullptr;
	RhiShaderProgram* pixel_shader = nullptr;
	RhiShaderProgram* geometry_shader = nullptr;
	RhiShaderProgram* hull_shader = nullptr;
	RhiShaderProgram* tess_shader = nullptr;
	RhiShaderProgram* domain_shader = nullptr;
	RhiShaderProgram* as_shader = nullptr;
	RhiShaderProgram* mesh_shader = nullptr;
};
class RhiDevice;
class RhiPipelineLayout;
class RhiRasterPipeline
	: public ICreateRhiObject<const RhiDevice&, const RhiPipelineLayout&,
	const RhiPipelineShaderPrograms&>
	, public RhiImpl<RHI_RASTER_PIPELINE> {
public:

	RhiRasterPipeline(RHI_RASTER_PIPELINE* handle = nullptr);
	virtual ~RhiRasterPipeline() = default;

	void create(const RhiDevice& device, const RhiPipelineLayout& layout,
		const RhiPipelineShaderPrograms& shader_programs);
	std::vector<RHI_INPUT_LAYOUT_DESC>& get_input_descriptors() { return m_input_descriptors; }
	void add_input_descriptor(const std::string& name, size_t offset, resource_format format);
private:
	std::vector<RHI_INPUT_LAYOUT_DESC> m_input_descriptors;
};

#endif // __rhi_raster_pipeline_hpp__
