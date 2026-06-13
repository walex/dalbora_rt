#include "dalbora_rt_api.hpp"
#include "tests/gltf_scene.hpp"

constexpr size_t surface_width = 800;
constexpr size_t surface_height = 600;

struct GPU_RESOUCES{

	RhiDevice device;
	RhiRenderTarget render_target;
	RhiView render_target_view;
	std::vector<RhiGraphicsCommandQueue> command_queues;
};
size_t k_gpu_count = 1;

Scene create_scene_from_file(const std::string& file_path, const RhiDevice& device, 
	RhiCommandBuffer& command_buffer) {

	Scene scene;

	// copy vertices e indices from scene
	scene.max_size = 6 * 1024 * 1024;
	scene.enable_rt_features(true);
	size_t tmp_buffer_offset = 0;
	size_t rt_buffer_index = 0;
	RhiSharedBuffer tmp_buffer;
	tmp_buffer.create(device, scene.max_size);
	command_buffer.record([&] {

		// scene load callback
		scene.on_geometry_loaded = ([&](Mesh& mesh, const std::string& attr,
			const uint8_t* const data, const size_t length, const size_t stride,
			const resource_format format) {

				// copy vertices from cpu visible memory to gpu
				if (data != nullptr) {
					tmp_buffer.copy(data, length, tmp_buffer_offset);
					if (attr == "POSITION") {
						mesh.vertex_buffer.create(device, length, stride, format);
						mesh.vertex_buffer.upload(command_buffer, tmp_buffer, tmp_buffer_offset, 0, length);
					}
					else if (attr == "__indices__") {

						mesh.index_buffer = std::make_unique<RhiGPUBuffer>();
						mesh.index_buffer->create(device, length, stride, format);
						mesh.index_buffer->upload(command_buffer, tmp_buffer, tmp_buffer_offset, 0, length);
					}
				}
				tmp_buffer_offset += length;
			});

		scene.on_model_loaded = ([&](const std::vector<Mesh*>& meshes) {
			std::vector<RHI_BUFFER*> vertices_ptr;
			std::vector<RHI_BUFFER*> indices_ptr;
			vertices_ptr.reserve(meshes.size());
			indices_ptr.reserve(meshes.size());
			for (auto& mesh : meshes) {
				vertices_ptr.push_back(mesh->vertex_buffer);
				if (mesh->index_buffer != nullptr)
					indices_ptr.push_back(mesh->vertex_buffer);
				else
					indices_ptr.push_back(nullptr);
			}
			scene.fill_rt_buffer(device, command_buffer, vertices_ptr, indices_ptr);
			});

		scene.on_new_scene_node = ([&](SceneNode& node) {

			for (auto& child : node.get_childs()) {
				if (child->is_leaf()
					&& static_cast<LeafNode*>(child.get())->get_type() == LeafNodeType_Mesh) {

					MeshNode* mesh_node = static_cast<MeshNode*>(child.get());
					scene.add_rt_instance_transform(mesh_node->mesh_index, mesh_node->get_world_transform().data());
				}
			}
			});

		// load scene
		load_gltf_scene(device,
			command_buffer,
			R"(C:\Users\wadrw\Documents\develop\projects\personal\rtx\models_3d\InteriorTest.obj.gltf)",
			//R"(C:\Users\wadrw\Documents\develop\projects\personal\rtx\models_3d\FinalBaseMesh.gltf)",
			//R"(C:\Users\wadrw\Documents\develop\projects\personal\rtx\models_3d\SheenChair.gltf)",
			0,
			scene);
		scene.create_rt_instances(device, command_buffer);
		});

	return scene;
}

void test_rt() {

	// ---- graphics device init ----

	constexpr resource_format surface_format = resource_format_R8G8B8A8_norm;
	std::vector<GPU_RESOUCES> gpu_resources(k_gpu_count);
	GPU_RESOUCES& gpu_resource = gpu_resources.at(0);

	RhiDevice& device = gpu_resource.device;
	RhiRenderTarget& render_target = gpu_resource.render_target;
	RhiView& render_target_view = gpu_resource.render_target_view;
	RhiGraphicsCommandQueue& command_queue = gpu_resource.command_queues.at(0);

	// init device
	rhi_init(device_type_dx12);
	device.create(INT64_MAX, device_features_raytracing);
	render_target.create(device, surface_format, surface_width, surface_height);
	render_target_view = render_target.new_rw_view(device);
	command_queue.create(device);

	// create command buffer
	RhiCommandBuffer command_buffer;
	command_buffer.create(device, command_queue);

	// load scene

	Scene scene;
	command_queue.sync_exec([&](RhiCommandQueueBufferList& list) {
		std::string scene_path = R"(C:\Users\wadrw\Documents\develop\projects\personal\rtx\models_3d\InteriorTest.obj.gltf)";
		scene = create_scene_from_file(scene_path, device, command_buffer);
		list.add_command_buffer(command_buffer);
	});

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
	std::string ray_gen_shader_file = R"(C:\Users\wadrw\Documents\develop\projects\personal\rtx\dalbora_rt\src\tests\ray_trace.hlsl)";
	std::string miss_shader_file = R"(C:\Users\wadrw\Documents\develop\projects\personal\rtx\dalbora_rt\src\tests\ray_trace.hlsl)";
	std::string closest_hit_shader_file = R"(C:\Users\wadrw\Documents\develop\projects\personal\rtx\dalbora_rt\src\tests\ray_trace.hlsl)";

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
