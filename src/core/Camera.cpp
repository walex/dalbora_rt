#include "Camera.hpp"
#include "Scene.hpp"
#include "Samples.hpp"

std::shared_ptr<Samples> Camera::generateSamples(const Eigen::Vector4i& UNUSED_PARAM(grid)) {

	return std::shared_ptr<Samples>();
}