#include "dalbora_rt_api.hpp"


constexpr size_t surface_width = 800;
constexpr size_t surface_height = 600;

struct GPU_RESOUCES{

	RhiDevice device;
	RhiRenderTarget render_target;
	RhiView render_target_view;
	std::vector<RhiGraphicsCommandQueue> command_queues;
};
size_t k_gpu_count = 1;



void test_rt() {

	// ---- graphics device init ----

	constexpr resource_format surface_format = resource_format_R8G8B8A8_norm;
	std::vector<GPU_RESOUCES> gpu_resources(k_gpu_count);
	GPU_RESOUCES& gpu_resource = gpu_resources.at(0);

	RhiDevice& device = gpu_resource.device;
	RhiRenderTarget& render_target = gpu_resource.render_target;
	RhiView& render_target_view = gpu_resource.render_target_view;
	RhiGraphicsCommandQueue& command_queue = gpu_resource.command_queues.emplace_back();

	// init device
	rhi_init(device_type_dx12);
	device.create(INT64_MAX, device_features_raytracing);
	render_target.create(device, surface_format, surface_width, surface_height);
	render_target_view = render_target.new_rw_view(device);
	command_queue.create(device);

	// load scene from file
	std::filesystem::path model_3d_folder = get_executable_folder("test_3d_models");
	std::string model_3d_file = model_3d_folder.concat("InteriorTest.obj.gltf").string();
	RayTraceScene scene;
	scene.set_max_size(6 * 1024 * 1024);
	scene.load(model_3d_file,
		device, command_queue);

	// ---- ray trace engine ----
	
	// light	
	//auto light = std::make_unique<AreaLight_Circular>(device);
	//light->setPosition(0, 1, 2);
	//light->setDirection(0, 1, 2);

	// sampler
	auto sampler = std::make_unique<Sampler>();

	// camera
	auto camera = std::make_unique<Camera>();
	camera->setSampler(sampler.get());
	camera->setPosition(0, 1, 2);
	camera->setLookAt(0, 1, 2);	

	// material
	auto material = std::make_unique<Material>(device);
	
	// scene
	//scene.addLight(light.get());
	//scene.addCamera(camera.get());

	// culler
	auto scene_culler = std::make_unique<SceneCuller>();
	scene_culler->setScene(&scene);

	// renderer
	auto renderer = std::make_unique<RayTracingRenderer>();

	// shaders
	std::filesystem::path shaders_folder = get_executable_folder("shaders");
	std::string ray_gen_shader_file = shaders_folder.concat("ray_trace.hlsl").string();
	std::string miss_shader_file = shaders_folder.concat("ray_trace.hlsl").string();
	std::string closest_hit_shader_file = shaders_folder.concat("ray_trace.hlsl").string();

	// compile shaders
	RhiShaderProgram ray_gen_shader;
	RhiShaderProgram miss_shader;
	RhiShaderProgram closest_hit_shader;

	ray_gen_shader.create(ray_gen_shader_file,
		"RayGen", "lib_6_6");
	miss_shader.create(miss_shader_file,
		"Miss", "lib_6_6");
	closest_hit_shader.create(closest_hit_shader_file,
		"ClosestHit", "lib_6_6");

	// ---- render stuff ----

	// cull scene geometries
	scene_culler->cull();
	auto geometries = scene_culler->getGeometries();
	auto lights = scene_culler->getLights();

	// ray trace and compute radiance	
	auto ray_integrator = std::make_unique<RayRadianceIntegratorGPU>(
		device, command_queue,
		ray_gen_shader, miss_shader,
		closest_hit_shader,
		surface_format);
	
	SurfaceRadianceGPU surface_radiance;
	std::vector<SurfaceRadiance*> out_radiances = { &surface_radiance };
	ray_integrator->run(*camera, geometries, lights, out_radiances);

	command_queue.sync_exec([&](RhiCommandQueueBufferList& list) {

		// draw
		renderer->setView(&render_target_view);
		renderer->draw(out_radiances);
		});

	// ---- graphics device end ----

	rhi_end();
}

int main(int UNUSED_PARAM(argc), char* UNUSED_PARAM(argv[])) {

	test_rt();

	return 0;
}
