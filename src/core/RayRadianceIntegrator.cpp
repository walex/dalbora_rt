#include "RayRadianceIntegrator.hpp"
#include "Culler.hpp"
#include "RayTracingRenderer.hpp"
#include "SurfaceRadiance.hpp"
#include "Camera.hpp"
#include "Rays.hpp"

Rays RayRadianceIntegrator::generateRays(const Camera& UNUSED_PARAM(camera), const Samples& UNUSED_PARAM(samples)) {

	return Rays();
}

void RayRadianceIntegrator::LI(const Rays& UNUSED_PARAM(rays), const std::vector<Mesh*>& UNUSED_PARAM(geometries),
	const std::vector<Mesh*>& UNUSED_PARAM(lights), const size_t tile[4], SurfaceRadiance& UNUSED_PARAM(radiance))
{
	// compute the radiance for the ray set and geometries

}