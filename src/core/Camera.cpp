#include "Camera.hpp"
#include "Scene.hpp"
#include "Samples.hpp"

BaseCamera::BaseCamera(RhiDevice& device) {
	m_transforms.create(device, sizeof(_BaseCamera));
	m_transforms_view = m_transforms.new_constant_buffer_view(device);
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