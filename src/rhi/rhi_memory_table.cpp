#include "rhi_memory_table.hpp"
#include "rhi_device.hpp"

RhiMemoryTable::RhiMemoryTable(const RhiDevice& device, const memory_descriptor_type type,
	const size_t descriptors_count, const std::vector<size_t>& slot_group_group_offsets)
	: RhiImpl<RHI_MEMORY_DESCRIPTOR>(nullptr) {

	ASSERT_EXPR(descriptors_count > 0, "Descriptors count must be greater than zero");
	this->create(device, type, descriptors_count);

	size_t slots_group_count = slot_group_group_offsets.size();
	if (slots_group_count == 0) {
		std::queue<size_t>& group_queue = m_descriptor_group_offsets.emplace_back();
		for (size_t j = 0; j < descriptors_count; ++j) {
			group_queue.push(j);
		}
	}
	else {
		size_t last_offset = 0;
		ASSERT_EXPR(descriptors_count >= std::accumulate(slot_group_group_offsets.begin(), slot_group_group_offsets.end(), 0), "Descriptors count does not match the sum of slot group offsets");
		m_descriptor_group_offsets.resize(slots_group_count);
		for (size_t i = 0; i < slots_group_count; ++i) {
			size_t elements = slot_group_group_offsets[i];
			std::queue<size_t>& group_queue = m_descriptor_group_offsets[i];
			for (size_t j = last_offset; j < last_offset + elements; ++j) {
				group_queue.push(j);
			}
			last_offset += elements;
		}
	}
}

RhiMemoryTable::~RhiMemoryTable() {
}

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

void RhiMemoryTable::next_descriptor_ptr(RHI_MEMORY_DESCRIPTOR_SLOT** slot, const size_t group_index) {
	ASSERT_EXPR(group_index < m_descriptor_group_offsets.size(), "Invalid group index");
	{
		std::lock_guard<std::mutex> lock(m_descriptor_group_mutex);
		std::queue<size_t>& group_queue = m_descriptor_group_offsets[group_index];
		ASSERT_EXPR(group_queue.size() > 0, "No more descriptors available in the group");
		size_t idx = group_queue.front();
		group_queue.pop();
		*slot = rhi_memory_resource_get_descriptor(*this, idx);
	}
}

RhiMemoryDescriptor* RhiMemoryTable::next_descriptor_ptr(const size_t group_index) {

	RHI_MEMORY_DESCRIPTOR_SLOT* slot = nullptr;
	this->next_descriptor_ptr(&slot, group_index);
	ASSERT_PTR(slot, "Failed to get next descriptor slot");
	return new RhiMemoryDescriptor(slot, *this, group_index);
}

std::unique_ptr<RhiMemoryDescriptor> RhiMemoryTable::next_descriptor(const size_t group_index) {
	return std::unique_ptr<RhiMemoryDescriptor>(this->next_descriptor_ptr(group_index));
}

void RhiMemoryTable::restore_descriptor_slot(const size_t slot_id, const size_t group_index) {
	ASSERT_EXPR(group_index < m_descriptor_group_offsets.size(), "Invalid group index");
	std::lock_guard<std::mutex> lock(m_descriptor_group_mutex);
	std::queue<size_t>& group_queue = m_descriptor_group_offsets[group_index];
	group_queue.push(slot_id);
}