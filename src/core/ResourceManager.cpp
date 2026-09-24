#include "ResourceManager.hpp"

constexpr size_t k_scene_buffers_descriptor_group_constant_buffers = 0;
constexpr size_t k_scene_buffers_descriptor_group_read_only_buffers = 1;
constexpr size_t k_scene_buffers_descriptor_group_rw_buffers = 2;
constexpr size_t k_scene_buffers_descriptor_group_count = 3;

ResourceManager::ResourceManager(RhiDevice& device)
	: m_device(device) {}

void ResourceManager::create_descriptor_tables(const size_t constants_buffers_size,
	const size_t read_only_buffers_size, const size_t rw_buffers_size) {

	const size_t descriptors_count = constants_buffers_size + read_only_buffers_size + rw_buffers_size;
	const std::vector<size_t> descriptor_groups = { constants_buffers_size, read_only_buffers_size, rw_buffers_size };
	
	m_buffers_memory_descriptors = std::make_unique<RhiMemoryTable>(m_device, memory_descriptor_type_buffer,
		descriptors_count, descriptor_groups);
	m_constants_buffers_size = constants_buffers_size;
	m_read_only_buffers_size = read_only_buffers_size;
	m_rw_buffers_size = rw_buffers_size;

	if (rhi_get_device_type() == device_type_dx12) {
		// create rtv memory descriptors (dx12 only)
		m_rtv_memory_descriptors = std::make_unique<RhiMemoryTable>(m_device, memory_descriptor_type_rtv,
			RTV_HEAP_DESCRIPTORS_COUNT);
	}
}

RhiMemoryDescriptor* ResourceManager::get_read_only_buffer_descriptor_slot() {

	return m_buffers_memory_descriptors->next_descriptor_ptr(k_scene_buffers_descriptor_group_read_only_buffers);
}

RhiMemoryDescriptor* ResourceManager::get_constant_buffer_descriptor_slot() {

	return m_buffers_memory_descriptors->next_descriptor_ptr(k_scene_buffers_descriptor_group_constant_buffers);
}

RhiMemoryDescriptor* ResourceManager::get_rw_buffer_descriptor_slot() {

	return m_buffers_memory_descriptors->next_descriptor_ptr(k_scene_buffers_descriptor_group_rw_buffers);
}

size_t ResourceManager::get_constant_buffer_descriptor_size() {
	
	return m_constants_buffers_size;
}

size_t ResourceManager::get_read_only_buffer_descriptor_size() {

	return m_read_only_buffers_size;
}

size_t ResourceManager::get_rw_buffer_descriptor_size() {

	return m_rw_buffers_size;
}
