#include "Camera.hpp"
#include "Scene.hpp"
#include "Samples.hpp"
#include "ResourceManager.hpp"

BaseCamera::BaseCamera(ResourceManager& rm) {
	m_transforms.create(rm.get_device(), sizeof(_BaseCamera), sizeof(float), resource_format_float);
	m_buffer_map = RhiSharedBufferMap(m_transforms, 0, sizeof(_BaseCamera));
	m_transforms.new_view(rm.get_device(), shader_view_type_constant_buffer, rm.get_constant_buffer_descriptor_slot());
}

BaseCamera::~BaseCamera() {
}

void BaseCamera::update(float dt) {
	memcpy(m_buffer_map.get_data(), static_cast<_BaseCamera*>(this), sizeof(_BaseCamera));
}

Samples Camera::generateSamples(const size_t grid[4]) const {

	return Samples();
}