#include "MosaicIntegrator.hpp"
#include "Culler.hpp"
#include "Render.hpp"
#include "SurfaceRadiance.hpp"
#include "Camera.hpp"
#include "Rays.hpp"

struct MosaicIntegratorWorkerParams: WorkerParams {

	Eigen::Vector4i tile;
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

std::vector<Eigen::Vector4i> MosaicIntegrator::generateTiles() {
	
	return std::vector<Eigen::Vector4i>();
}

std::shared_ptr<SurfaceRadiance> MosaicIntegrator::run(const std::shared_ptr<Camera> camera,
	const std::vector< std::shared_ptr<Mesh>>& geometries,
	const std::vector< std::shared_ptr<Mesh>>& lights)
{
	// create output surface
	auto radiance = std::make_shared<SurfaceRadiance>(); // FixME: should be a circular buffer to avoid memory allocation overhead
	// get tile coords
	std::vector<Eigen::Vector4i> tiles = this->generateTiles();
	// create parameters array
	std::vector<std::shared_ptr<MosaicIntegratorTaskParams>> task_tile_parameters;
	task_tile_parameters.reserve(tiles.size());
	for (size_t i = 0; i < tiles.size(); ++i) {
		// set up worker task
		auto taskParams = task_tile_parameters.emplace_back(std::make_shared<MosaicIntegratorTaskParams>());
		taskParams->tile = tiles[i];
		taskParams->camera = camera;
		taskParams->geometries = geometries;
		taskParams->lights = lights;
		taskParams->radiance = radiance;
		mMosaicIntegratorTaskPool->newTask(taskParams);
		// wait for a worker to take task or system exit
		while (taskParams->isRunnig.load() == false)
			std::this_thread::yield();
	}
	// wait for all tasks to end or system exit
	for (size_t i = 0; i < task_tile_parameters.size(); ++i)
		while (task_tile_parameters[i]->isDone.load() == false)
			std::this_thread::yield();
	// return final image
	return radiance;
}

void MosaicIntegrator::onLITask(std::shared_ptr<MosaicIntegratorTaskParams> taskParams) {

	const auto& camera = taskParams->camera;
	taskParams->isRunnig.store(true);
	// generate samples
	auto samples = camera->generateSamples(taskParams->tile);
	// generate rays
	auto rays = this->generateRays(camera, samples);
	// LI
	this->LI(rays, taskParams->geometries, taskParams->lights, taskParams->tile, taskParams->radiance);
	taskParams->isDone.store(true);
}