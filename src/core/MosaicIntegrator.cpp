#include "MosaicIntegrator.hpp"
#include "Culler.hpp"
#include "RayTracingRenderer.hpp"
#include "SurfaceRadiance.hpp"
#include "Camera.hpp"
#include "Rays.hpp"
#include "Samples.hpp"

struct MosaicIntegratorWorkerParams: WorkerParams {

	size_t tile[4];
	MosaicIntegrator* integrator;
};

MosaicIntegratorTaskPool::MosaicIntegratorTaskPool(MosaicIntegrator* integrator, unsigned int core_count)
	: mIntegrator(integrator)
	, TaskPool<std::shared_ptr<MosaicIntegratorTaskParams>>(core_count) {
	
}


void MosaicIntegratorTaskPool::onTask(const std::shared_ptr<MosaicIntegratorTaskParams> data) {

	mIntegrator->onLITask(data);
}


MosaicIntegrator::MosaicIntegrator()
	: mMosaicIntegratorTaskPool(std::make_unique<MosaicIntegratorTaskPool>(this, std::thread::hardware_concurrency())) {

}

std::vector<size_t[4]> MosaicIntegrator::generateTiles(size_t count) {
	
	return std::vector<size_t[4]>(count);
}

std::shared_ptr<MosaicIntegratorTaskParams> MosaicIntegrator::create_params() {
	
	return std::make_shared<MosaicIntegratorTaskParams>();
}

void MosaicIntegrator::run(
	const Camera& camera,
	const std::vector<Mesh*>& geometries,
	const std::vector<Mesh*>& lights,
	const std::vector<SurfaceRadiance*>& out_radiances)
{
	// create output surface
	// get tile coords
	std::vector<size_t[4]> tiles = this->generateTiles(out_radiances.size());
	std::vector<std::shared_ptr<MosaicIntegratorTaskParams>> task_tile_parameters;
	task_tile_parameters.reserve(tiles.size());
	for (size_t i = 0; i < tiles.size(); ++i) {
		// set up worker task
		auto taskParams = this->create_params();
		memcpy(taskParams->tile, &tiles[i], sizeof(size_t[4]));
		taskParams->camera = &camera;
		taskParams->geometries = &geometries;
		taskParams->lights = &lights;
		taskParams->radiance = out_radiances[i];
		mMosaicIntegratorTaskPool->newTask(taskParams);
		// wait for a worker to take task or system exit
		while (taskParams->isRunnig.load() == false)
			std::this_thread::yield();
	}
	// wait for all tasks to end or system exit
	for (size_t i = 0; i < task_tile_parameters.size(); ++i)
		while (task_tile_parameters[i]->isDone.load() == false)
			std::this_thread::yield();
}
 
void MosaicIntegrator::onLITask(std::shared_ptr<MosaicIntegratorTaskParams> taskParams) {

	const Camera* camera = taskParams->camera;
	taskParams->isRunnig.store(true);
	// generate samples
	Samples samples = camera->generateSamples(taskParams->tile);
	auto rays = this->generateRays(*camera, samples);
	// LI
	this->LI(rays, *taskParams->geometries, *taskParams->lights, taskParams->tile, *taskParams->radiance);
	taskParams->isDone.store(true);
}