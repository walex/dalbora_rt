#include "Camera.hpp"
#include "Scene.hpp"
#include "Samples.hpp"
#include "ResourceManager.hpp"

BaseCamera::BaseCamera(ResourceManager& rm) {
	m_transforms.create(rm.get_device(), sizeof(CameraBuffer));
	m_buffer_map = std::make_unique<RhiSharedBufferMap>(m_transforms, 0, sizeof(CameraBuffer));
	m_camera_transform_view = m_transforms.new_view(rm.get_device(), shader_view_type_constant_buffer, rm.get_constant_buffer_descriptor_slot());
}

BaseCamera::~BaseCamera() {
}

void BaseCamera::update(float dt) {

	memcpy(m_buffer_map->get_data(), static_cast<CameraBuffer*>(this), sizeof(CameraBuffer));
}

Samples Camera::generateSamples(const size_t grid[4]) const {

	return Samples();
}