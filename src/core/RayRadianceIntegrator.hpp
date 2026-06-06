#ifndef __RayRadianceIntegrator_h__
#define __RayRadianceIntegrator_h__

#include "Common.hpp"
#include "MosaicIntegrator.hpp"

// CPU based ray tracing integrator

class RayRadianceIntegrator : public MosaicIntegrator
{
protected:
	Rays generateRays(const Camera& camera, const Samples& samples) override;
	void LI(const Rays& rays, const std::vector<Mesh*>& geometries,
		const std::vector<Mesh*>& lights, const size_t tile[4], SurfaceRadiance& out_radiance) override;
};

#endif
