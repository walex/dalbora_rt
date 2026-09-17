#include "ResourceManager.hpp"
//
//ResourceManager::ResourceManager(const RhiDevice& device, const std::vector<size_t>& space_slots)
//	: m_space_slots(space_slots) {
//
//	size_t heap_slots_count = std::accumulate(m_space_slots.begin(), m_space_slots.end(), 0);
//	m_descriptor_heap.create(device, memory_descriptor_type_buffer, heap_slots_count);
//}
//

RhiMemoryTable* ResourceManager::g_memory_descriptor = nullptr;

void ResourceManager::set_memory_descriptor(RhiMemoryTable* memory_descriptor) {
	ResourceManager::g_memory_descriptor = memory_descriptor;
}

RhiMemoryTable& ResourceManager::get_memory_descriptor() {
	ASSERT_PTR(ResourceManager::g_memory_descriptor);
	return *ResourceManager::g_memory_descriptor;
}

RhiView ResourceManager::new_resource_view(const RhiDevice& device, const RhiBuffer& buffer ) {

	//ASSERT_PTR(g_memory_descriptor);

	//RHI_VIEW_DESC desc;
	//desc.device = device;
	//desc.buffer = buffer;
	//desc.type = buffer.get_type();
	//desc.format = buffer.get_format();
	//desc.memory_descriptor = g_memory_descriptor->next_descriptor().get();
	//return RhiView(rhi_buffers_create_view(&desc), desc.memory_descriptor->slot_id);

	return RhiView();
}