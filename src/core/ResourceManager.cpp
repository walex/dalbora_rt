#include "ResourceManager.hpp"

ResourceManager::ResourceManager(const RhiDevice& device, const std::vector<size_t>& space_slots)
	: m_space_slots(space_slots) {

	size_t heap_slots_count = std::accumulate(m_space_slots.begin(), m_space_slots.end(), 0);
	m_descriptor_heap.create(device, resource_type_rw_shader_buffer, heap_slots_count);
}

RhiView ResourceManager::new_resource_view(const RhiDevice& device, const RhiBuffer& buffer ) {

	RHI_VIEW_DESC desc;
	desc.device = device;
	desc.buffer = buffer;
	desc.type = buffer.get_type();
	desc.format = buffer.get_format();
	RhiDescriptorHeapResource slot = m_descriptor_heap.next_heap_slot();
	return RhiView(rhi_buffers_create_view(&desc), std::move(slot));
}