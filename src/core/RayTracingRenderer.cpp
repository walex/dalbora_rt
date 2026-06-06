#include "RayTracingRenderer.hpp"
#include "SurfaceRadiance.hpp"

RayTracingRenderer::RayTracingRenderer() {
	
	
}

void RayTracingRenderer::draw(const std::vector<SurfaceRadiance*>& radiances)
{
	// compose and draw the surface radiance
	for (auto& radiance : radiances) {

		//RhiGPUBuffer gpu_input_buffer = radiance;
		//render_target.upload_to_region(gpu_input_buffer, radiance.get_region());
	}
}