#include "rhi_raster_pipeline.hpp"
#include "rhi.hpp"

RhiRasterPipeline::RhiRasterPipeline(RHI_RASTER_PIPELINE* handle)
: RhiPipeline<RHI_RASTER_PIPELINE>(handle){}

void RhiRasterPipeline::create(const RhiDevice& device, const RhiPipelineLayout& layout,
	const RhiRasterPipelineShaderPrograms& shader_programs, RHI_MEMORY_DESCRIPTOR& memory_descriptor) {
		
	RHI_RASTER_PIPELINE_DESC desc;
	desc.device = device;
	desc.layout = layout;
	desc.input_layouts_desc_count = m_input_descriptors.size();
	memcpy(&desc.input_layouts_desc[0], m_input_descriptors.data(), m_input_descriptors.size() * sizeof(RHI_INPUT_LAYOUT_DESC));
	if (shader_programs.vertex_shader) {
		desc.vertex_shader = static_cast<RHI_COMPILED_SHADER_BUFFER*>(*shader_programs.vertex_shader);
		strcpy(desc.vertex_shader_entry_point, shader_programs.vertex_shader->get_entry_point());
	}
	if (shader_programs.pixel_shader) {
		desc.pixel_shader = static_cast<RHI_COMPILED_SHADER_BUFFER*>(*shader_programs.pixel_shader);
		strcpy(desc.pixel_shader_entry_point, shader_programs.pixel_shader->get_entry_point());
	}
	desc.topology = layout.get_topology();
	desc.format = layout.get_format();
	desc.depth_buffer_format = layout.get_depth_buffer_format();
	desc.vertex_stride = shader_programs.vertex_stride;
	desc.index_stride = shader_programs.index_stride;
	desc.resources_memory_descriptor = &memory_descriptor;
	this->set_handle(rhi_raster_pipeline_create(&desc));
	
	
}

void RhiRasterPipeline::add_input_descriptor(const std::string& name, size_t offset, resource_format format) {

	auto& id = m_input_descriptors.emplace_back();
	strcpy_s(id.name, MAX_INPUT_NAME_LENGTH, name.c_str());
	id.offset = offset;
	id.format = format;
}