#ifndef __MosaicIntegrator_hpp__
#define __MosaicIntegrator_hpp__

#include "Common.hpp"

struct MosaicIntegratorWorkerParams; // forward declaration added
class Camera;
class Render;
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
	std::shared_ptr<SurfaceRadiance> run(std::shared_ptr<Camera> camera,
		const std::vector< std::shared_ptr<Mesh>>& geometries,
		const std::vector< std::shared_ptr<Mesh>>& lights);
	void onLITask(std::shared_ptr<MosaicIntegratorTaskParams> taskParams);
protected:
	virtual std::shared_ptr<Rays> generateRays(std::shared_ptr<Camera> camera, std::shared_ptr<Samples> samples) = 0;
	virtual void LI(std::shared_ptr<Rays> rays, const std::vector< std::shared_ptr<Mesh>>& geometries,
		const std::vector< std::shared_ptr<Mesh>>& lights, const Eigen::Vector4i& tile, std::shared_ptr<SurfaceRadiance> radiance) = 0;
private:
	std::vector<Eigen::Vector4i> generateTiles();
private:
	std::unique_ptr<MosaicIntegratorTaskPool> mMosaicIntegratorTaskPool;
};

struct MosaicIntegratorTaskParams {
	Eigen::Vector4i tile;
	std::shared_ptr<Camera> camera;
	std::vector< std::shared_ptr<Mesh>> geometries;
	std::vector< std::shared_ptr<Mesh>> lights;
	std::shared_ptr<SurfaceRadiance> radiance;
	std::atomic<bool> isRunnig{ false };
	std::atomic<bool> isDone{ false };
};

#endif
