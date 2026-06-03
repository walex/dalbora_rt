#include "rhi_raster_pipeline.hpp"
#include "rhi_device.hpp"
#include "rhi_pipeline_layout.hpp"
#include "rhi_shader_program.hpp"

RhiRasterPipeline::RhiRasterPipeline(RHI_RASTER_PIPELINE* handle)
: RhiImpl<RHI_RASTER_PIPELINE>(handle){}

void RhiRasterPipeline::create(const RhiDevice& device, const RhiPipelineLayout& layout,
	const RhiPipelineShaderPrograms& shader_programs) {
		
	RHI_RASTER_PIPELINE_DESC desc;
	desc.device = device;
	desc.layout = layout;
	desc.input_layouts_desc_count = m_input_descriptors.size();
	memcpy(&desc.input_layouts_desc[0], m_input_descriptors.data(), m_input_descriptors.size() * sizeof(RHI_INPUT_LAYOUT_DESC));
	desc.vertex_shader = shader_programs.vertex_shader  
		? static_cast<RHI_COMPILED_SHADER_BUFFER*>(*shader_programs.vertex_shader) 
		: nullptr;
	desc.pixel_shader = shader_programs.pixel_shader 
		? static_cast<RHI_COMPILED_SHADER_BUFFER*>(*shader_programs.pixel_shader)
		: nullptr;
	desc.topology = layout.get_topology();
	desc.format = layout.get_format();
	desc.depth_buffer_format = layout.get_depth_buffer_format();
	this->set_handle(rhi_raster_pipeline_create(&desc));
}

void RhiRasterPipeline::add_input_descriptor(const std::string& name, size_t offset, resource_format format) {

	auto& id = m_input_descriptors.emplace_back();
	strcpy(id.name, name.c_str());
	id.offset = offset;
	id.format = format;
}