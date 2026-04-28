#include "Common.hpp"
#include "Scene.hpp"
#include "Mesh.hpp"
#include "Camera.hpp"
#include "Light.hpp"
#include "Material.hpp"
#include "SceneCuller.hpp"
#include "Render.hpp"
#include "RayRadianceIntegrator.hpp"
#include "SurfaceRadiance.hpp"
#include "Sampler.hpp"
#include "AreaLight_Circular.hpp"
#include "rhi.hpp"

void test_graphics_api() {

	rhi_init(device_type_dx12);

	RHI_WINDOW_DESC window_desc;
	window_desc.full_screen = false;
	window_desc.width = 800;
	window_desc.height = 600;
	strcpy_s(window_desc.title, "pbr test");
	auto window = rhi_create_window(window_desc);

	RHI_DEVICE_DESC device_desc;
	device_desc.adapter_id = 0;
	device_desc.features = device_features_raytracing;
	auto dev = rhi_create_device(device_desc);

	RHI_COMMAND_QUEUE_DESC queue_desc;
	queue_desc.device = dev.get();
	auto queue = rhi_create_graphics_command_queue(queue_desc);

	RHI_SWAP_CHAIN_DESC swap_chain_desc;
	swap_chain_desc.width = 800;
	swap_chain_desc.height = 600;
	swap_chain_desc.allow_tearing = false;
	swap_chain_desc.buffer_count = 2;
	swap_chain_desc.color_format = resource_format_R8G8B8A8;
	swap_chain_desc.device = dev.get();
	swap_chain_desc.command_queue = queue.get();
	swap_chain_desc.window = window.get();
	auto swap_chain = rhi_create_swap_chain(swap_chain_desc);

	rhi_end();
}

void test_rt() {

	auto scene = std::make_shared<Scene>();
	auto light = std::make_shared<AreaLight_Circular>();
	auto camera = std::make_shared<Camera>();
	auto geometry = std::make_shared<Mesh>();
	auto material = std::make_shared<Material>();
	auto scene_culler = std::make_shared<SceneCuller>();
	auto render = std::make_shared<Render>();
	auto ray_integrator = std::make_shared<RayRadianceIntegrator>();
	auto sampler = std::make_shared<Sampler>();

	// ---- scene stuff ----

	// scene
	scene->addLight(light);
	scene->addCamera(camera);
	scene->addGeometry(geometry);

	// camera
	camera->setSampler(sampler);
	camera->setPosition(0, 1, 2);
	camera->setLookAt(0, 1, 2);

	// geometry
	geometry->setMaterial(material);

	// light	
	light->setPosition(0, 1, 2);
	light->setDirection(0, 1, 2);

	// culler
	scene_culler->setScene(scene);

	// ---- render stuff ----

	// cull scene geometries
	scene_culler->cull();
	auto geometries = scene_culler->getGeometries();
	auto lights = scene_culler->getLights();

	// ray trace and compute radiance
	auto radiance = ray_integrator->run(camera, geometries, lights);

	// draw
	render->draw(radiance);
}

int main(int UNUSED_PARAM(argc), char* UNUSED_PARAM(argv[])) {

	test_graphics_api();	

	return 0;
}
