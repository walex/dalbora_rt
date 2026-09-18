#include "rhi_memory_table.hpp"
#include "rhi_device.hpp"

void RhiMemoryTable::create(const RhiDevice& device, const memory_descriptor_type type,
	const size_t descriptors_count) {

	RHI_MEMORY_RESOURCE_DESC desc = {};
	desc.device = device;
	desc.type = memory_resource_type_descriptor_table;
	desc.descriptor_desc.type = type;
	desc.descriptor_desc.count = descriptors_count;
	desc.descriptor_desc.shader_visible = true;
	this->set_handle(rhi_memory_resource_create(&desc));
}

std::unique_ptr<RHI_MEMORY_DESCRIPTOR_SLOT> RhiMemoryTable::next_descriptor(size_t group_index) const {

	ASSERT_EXPR(group_index < m_descriptor_group_offsets.size(), "Invalid group index");
	size_t idx = m_descriptor_group_offsets[group_index]++;
	RHI_MEMORY_DESCRIPTOR_SLOT* descriptor = rhi_memory_resource_get_descriptor(*this, idx);
	return std::unique_ptr<RHI_MEMORY_DESCRIPTOR_SLOT>(descriptor);
}

RHI_MEMORY_DESCRIPTOR_SLOT* RhiMemoryTable::next_descriptor_ptr(size_t group_index) const {
	return this->next_descriptor(group_index).release();
}