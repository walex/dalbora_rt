#ifndef __RayRadianceIntegrator_h__
#define __RayRadianceIntegrator_h__

#include "Common.hpp"
#include "MosaicIntegrator.hpp"

// CPU based ray tracing integrator

class RayRadianceIntegrator : public MosaicIntegrator
{
protected:
	std::shared_ptr<Rays> generateRays(std::shared_ptr<Camera> camera, std::shared_ptr<Samples> samples) override;
	void LI(std::shared_ptr<Rays> rays, const std::vector< std::shared_ptr<Mesh>>& geometries,
		const std::vector< std::shared_ptr<Mesh>>& lights, const Eigen::Vector4i& tile, std::shared_ptr<SurfaceRadiance> radiance) override;
};

#endif
