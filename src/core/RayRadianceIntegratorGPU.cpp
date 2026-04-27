#include "RayRadianceIntegratorGPU.hpp"
#include "Culler.hpp"
#include "Render.hpp"
#include "SurfaceRadiance.hpp"
#include "Camera.hpp"
#include "Rays.hpp"


std::shared_ptr<Rays> RayRadianceIntegratorGPU::generateRays(std::shared_ptr<Camera> UNUSED_PARAM(camera), std::shared_ptr<Samples> UNUSED_PARAM(samples)) {

	//TODO: call ray generation on gpu
	return std::make_shared<Rays>();
}

void RayRadianceIntegratorGPU::LI(std::shared_ptr<Rays> UNUSED_PARAM(rays), const std::vector<std::shared_ptr<Mesh>>& UNUSED_PARAM(geometries),
	const std::vector<std::shared_ptr<Mesh>>& UNUSED_PARAM(lights), const Eigen::Vector4i& UNUSED_PARAM(tile), std::shared_ptr<SurfaceRadiance> UNUSED_PARAM(radiance)) {
	
	// LI should be implemented on gpu
}