#include "Camera.hpp"
#include "Scene.hpp"
#include "Samples.hpp"
#include "ResourceManager.hpp"

BaseCamera::BaseCamera(RhiDevice& device) {
	m_transforms.create(device, sizeof(_BaseCamera), sizeof(float), resource_format_float);
	m_transforms_view = m_transforms.new_view(device, ResourceManager::get_memory_descriptor(), shader_view_type_constant_buffer);
	//m_transforms_view = m_transforms.new_shader_read_only_view(device, memory_descriptor);
	m_buffer_map = RhiSharedBufferMap(m_transforms, 0, sizeof(_BaseCamera));
}

BaseCamera::~BaseCamera() {
}

void BaseCamera::update(float dt) {
	memcpy(m_buffer_map.get_data(), static_cast<_BaseCamera*>(this), sizeof(_BaseCamera));
}

Samples Camera::generateSamples(const size_t grid[4]) const {

	return Samples();
}