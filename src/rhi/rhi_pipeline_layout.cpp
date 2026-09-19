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

void RhiPipelineLayout::set_shader_buffers_descriptor_offsets(const size_t cb_offset, const size_t cb_count,
	const size_t rd_offset, const size_t rd_count,
	const size_t rw_offset, const size_t rw_count,
	const size_t space_index) {

	this->add_descriptors(shader_view_type_constant_buffer, cb_offset, cb_count, space_index);
	this->add_descriptors(shader_view_type_read_only_buffer, rd_offset, rd_count, space_index);
	this->add_descriptors(shader_view_type_rw_buffer, rw_offset, rw_count, space_index);
}

void RhiPipelineLayout::set_shader_sampler_descriptor_offset(const size_t offset, const size_t count, const size_t space_index) {
	this->add_descriptors(shader_view_type_sampler, offset, count, space_index);
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

void RhiPipelineLayout::add_descriptors(const shader_view_type rt, const size_t offset, 
	const size_t count, const size_t space_index) {

	if (count == 0)
		return;

	RHI_SHADER_DESCRIPTOR_DESC& cbd = m_layout_desc.descriptors[m_layout_desc.descriptor_count++];
	cbd.shader_view_type = rt;
	cbd.shader_register_start = offset;
	cbd.shader_register_max = count;
	cbd.space_index = space_index;
}