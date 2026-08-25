#include "rhi_pipeline_layout.hpp"
#include "rhi.hpp"

RhiPipelineLayout::RhiPipelineLayout(RHI_PIPELINE_LAYOUT* handle)
	: RhiImpl<RHI_PIPELINE_LAYOUT>(handle)
	, m_layout_desc({})  {}

void RhiPipelineLayout::create(const RhiDevice& device, const primitive_topology topology,
	const resource_format surface_format, const resource_format depth_buffer_format) {
	
	m_layout_desc.device = device;
	m_layout_desc.topology = topology;
	m_layout_desc.surface_format = surface_format;
	m_layout_desc.depth_buffer_format = depth_buffer_format;
	this->set_handle(rhi_pipeline_layout_create(&m_layout_desc));
}

void RhiPipelineLayout::add_constants_buffer_descriptors(const size_t offset, const size_t count
	, const size_t space_index) {
	this->add_descriptors(resource_type_constant_buffer, offset, count, space_index);
}

void RhiPipelineLayout::add_read_only_buffer_descriptors(const size_t offset, const size_t count
	, const size_t space_index) {
	this->add_descriptors(resource_type_read_only_shader_buffer, offset, count, space_index);
}

void RhiPipelineLayout::add_rw_buffer_descriptors(const size_t offset, const size_t count, const size_t space_index) {
	this->add_descriptors(resource_type_rw_shader_buffer, offset, count, space_index);
}

void RhiPipelineLayout::add_sampler_buffer_descriptors(const size_t offset, const size_t count, const size_t space_index) {
	this->add_descriptors(resource_type_sampler, offset, count, space_index);
}

primitive_topology RhiPipelineLayout::get_topology() const {
	return static_cast<RHI_PIPELINE_LAYOUT*>(*this)->topology;
}

resource_format RhiPipelineLayout::get_format() const {
	return static_cast<RHI_PIPELINE_LAYOUT*>(*this)->surface_format;
}

resource_format RhiPipelineLayout::get_depth_buffer_format() const {
	return static_cast<RHI_PIPELINE_LAYOUT*>(*this)->depth_buffer_format;
}

void RhiPipelineLayout::add_descriptors(const resource_type rt, const size_t offset, 
	const size_t count, const size_t space_index) {

	RHI_SHADER_DESCRIPTOR_DESC& cbd = m_layout_desc.descriptors[m_layout_desc.descriptor_count++];
	cbd.resource_type = rt;
	cbd.shader_register_start = offset;
	cbd.shader_register_max = count;
	cbd.space_index = space_index;
}