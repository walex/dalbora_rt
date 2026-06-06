#ifndef __MosaicIntegrator_hpp__
#define __MosaicIntegrator_hpp__

#include "Common.hpp"

struct MosaicIntegratorWorkerParams; // forward declaration added
class Camera;
class RayTracingRenderer;
class SurfaceRadiance;
class Mesh;
class Rays;
class MosaicIntegrator;
class Samples;
struct MosaicIntegratorTaskParams;

class MosaicIntegratorTaskPool : public TaskPool<std::shared_ptr<MosaicIntegratorTaskParams>> {

public:
	MosaicIntegratorTaskPool(MosaicIntegrator* integrator, unsigned int core_count);

	void onTask(const std::shared_ptr<MosaicIntegratorTaskParams> data);
private:
	MosaicIntegrator* mIntegrator;
};

class MosaicIntegrator
{
public:
	MosaicIntegrator();
	virtual ~MosaicIntegrator() = default;
	virtual void run(
		const Camera& camera,
		const std::vector<Mesh*>& geometries,
		const std::vector<Mesh*>& lights,
		const std::vector<SurfaceRadiance*>& out_radiances);
	virtual void onLITask(std::shared_ptr<MosaicIntegratorTaskParams> taskParams);
protected:
	virtual std::shared_ptr<MosaicIntegratorTaskParams> create_params();
	virtual Rays generateRays(const Camera& camera, const Samples& samples) = 0;
	virtual void LI(const Rays& rays, const std::vector<Mesh*>& geometries,
		const std::vector<Mesh*>& lights, const size_t tile[4], SurfaceRadiance& out_radiance) = 0;
private:
	std::vector<size_t[4]> generateTiles(size_t count);
	std::unique_ptr<MosaicIntegratorTaskPool> mMosaicIntegratorTaskPool;
};

struct MosaicIntegratorTaskParams {
	size_t tile[4];
	const Camera* camera;
	const std::vector<Mesh*>* geometries;
	const std::vector<Mesh*>* lights;
	SurfaceRadiance* radiance;
	std::atomic<bool> isRunnig{ false };
	std::atomic<bool> isDone{ false };
};

#endif
