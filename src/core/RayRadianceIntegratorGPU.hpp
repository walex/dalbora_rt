#ifndef __RayRadianceIntegratorGPU_h__
#define __RayRadianceIntegratorGPU_h__

#include "Common.hpp"
#include "MosaicIntegrator.hpp"

struct RayRadianceIntegratorGPUTaskParams: public MosaicIntegratorTaskParams {

};

// GPU based ray tracing integrator

class RayRadianceIntegratorGPU : public MosaicIntegrator
{
public:
	RayRadianceIntegratorGPU(
		const RhiDevice& device, RhiCommandQueue& command_queue,
		const RhiShaderProgram& ray_gen_shader, const RhiShaderProgram& miss_shader,
		const RhiShaderProgram& closest_hit_shader, resource_format surface_format);
	virtual ~RayRadianceIntegratorGPU() = default;
	void run(
		const Camera& camera,
		const std::vector<Mesh*>& geometries,
		const std::vector<Mesh*>& lights,
		const std::vector<SurfaceRadiance*>& out_radiances) override;
protected:
	std::shared_ptr<MosaicIntegratorTaskParams> create_params() override;
	Rays generateRays(const Camera& camera, const Samples& samples) override;
	void LI(const Rays& rays, const std::vector<Mesh*>& geometries,
		const std::vector<Mesh*>& lights, const size_t tile[4], SurfaceRadiance& out_radiance) override;
private:
	const RhiShaderProgram* m_ray_gen_shader;
	const RhiShaderProgram* m_miss_shader;
	const RhiShaderProgram* m_closest_hit_shader;
	RhiCommandQueue* m_command_queue;
	RhiCommandBuffer m_command_buffer;
	RhiPipelineLayout m_pipeline_layout;
	RhiRayTracePipeline m_pipeline;
	RhiShaderBindingTable m_sbt;
};

#endif
