#include "RayRadianceIntegrator.hpp"
#include "Culler.hpp"
#include "Render.hpp"
#include "SurfaceRadiance.hpp"
#include "Camera.hpp"
#include "Rays.hpp"

std::shared_ptr<Rays> RayRadianceIntegrator::generateRays(std::shared_ptr<Camera> UNUSED_PARAM(camera), std::shared_ptr<Samples> UNUSED_PARAM(samples)) {

	return std::make_shared<Rays>();
}

void RayRadianceIntegrator::LI(std::shared_ptr<Rays> UNUSED_PARAM(rays), const std::vector<std::shared_ptr<Mesh>>& UNUSED_PARAM(geometries),
	const std::vector<std::shared_ptr<Mesh>>& UNUSED_PARAM(lights), const Eigen::Vector4i& UNUSED_PARAM(tile), std::shared_ptr<SurfaceRadiance> UNUSED_PARAM(radiance))
{
	// compute the radiance for the ray set and geometries

}