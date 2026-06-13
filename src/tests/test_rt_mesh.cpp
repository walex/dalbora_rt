#include "test_api.hpp"
#include "gltf_scene.hpp"

static constexpr float image_aspect = 800.0f / 600.0f;

void test_rt_mesh_obj(RhiUnitTestCallbacks* callbacks) {


	RhiRenderTarget render_target;
	RhiView render_target_view;
	RhiShaderProgram ray_gen_shader;
	RhiShaderProgram miss_shader;
	RhiShaderProgram closest_hit_shader;
	RhiGPUBuffer vertex_buffer;
	RhiGPUBuffer index_buffer;
	RhiSharedBuffer camera_transforms;
	RhiView camera_transform_view;
	RhiRayTraceRenderPass rt_render_pass;
	RhiShaderBindingTable sbt;
	Scene scene;

	std::unique_ptr<RhiSharedBufferMap> camera_constant_buffer_map;
	CameraCBRT camera_matrices;

	RhiUnitTestCallbacks unit_test_callbacks;
	unit_test_callbacks.on_init = ([&](RhiUnitTest& unit_test) {

		RhiWindow& window = unit_test.window;
		RhiDevice& device = unit_test.device;
		RhiGraphicsCommandQueue& command_queue = unit_test.command_queue;
		RhiCommandBuffer& command_buffer = unit_test.command_buffer;
		RhiSwapChain& swap_chain = unit_test.swap_chain;
		RhiPipelineLayout& pipeline_layout = unit_test.pipeline_layout;
		RhiRayTracePipeline& pipeline = unit_test.ray_trace_pipeline;

		// setup shaders
		unit_test.ray_gen_shader_file = R"(C:\Users\wadrw\Documents\develop\projects\personal\rtx\dalbora_rt\src\tests\simple_rt.hlsl)";
		unit_test.miss_shader_file = R"(C:\Users\wadrw\Documents\develop\projects\personal\rtx\dalbora_rt\src\tests\simple_rt.hlsl)";
		unit_test.closest_hit_shader_file = R"(C:\Users\wadrw\Documents\develop\projects\personal\rtx\dalbora_rt\src\tests\simple_rt.hlsl)";

		// create render target
		render_target.create(device, swap_chain.get_format(),
			window.get_width(), window.get_height());

		if (callbacks)
			callbacks->on_init(unit_test);

		// compile shaders
		ray_gen_shader.create(unit_test.ray_gen_shader_file,
			"RayGen", "lib_6_6");
		miss_shader.create(unit_test.miss_shader_file,
			"Miss", "lib_6_6");
		closest_hit_shader.create(unit_test.closest_hit_shader_file,
			"ClosestHit", "lib_6_6");

		// add layout descriptors ( order mathers )

		// 1 - GPU read only (Scene BVH)
		pipeline_layout.add_read_only_buffer_descriptors(0, 100);

		// 2 - GPU read write (Render Target)
		pipeline_layout.add_rw_buffer_descriptors(0, 100);

		// 3 - Constant buffer (Camera)
		pipeline_layout.add_constants_buffer_descriptors(0, 100);

		// create pipeline layout
		pipeline_layout.create(device, primitive_topology_triangle, swap_chain.get_format(), resource_format_d24_norm_s8_uint);

		// rt pipeline config
		std::string ray_gen_entry_point = "RayGen";
		std::string miss_entry_point = "Miss";
		std::string closest_hit_entry_point = "ClosestHit";

		// config ray trace shader
		RhiRayTracePipelineShaderPrograms ray_trace_shader_programs;
		std::vector<RHI_RT_HIT_GROUP_DESC>& hit_groups_desc = ray_trace_shader_programs.hit_groups_desc;
		auto& hg = hit_groups_desc.emplace_back();
		strcpy_s(hg.name_id, "HG_1");
		hg.closest_hit.blob = closest_hit_shader;
		strcpy_s(hg.closest_hit.name_id, closest_hit_entry_point.c_str());

		std::vector<RHI_RT_SHADER_UNIT_DESC>& miss_shader_desc = ray_trace_shader_programs.miss_shaders_desc;
		auto& miss_1 = miss_shader_desc.emplace_back();
		miss_1.blob = miss_shader;
		strcpy_s(miss_1.name_id, miss_entry_point.c_str());

		std::vector<RHI_RT_SHADER_UNIT_DESC>& ray_gen_shader_desc = ray_trace_shader_programs.ray_gen_shaders_desc;
		auto& ray_gen = ray_gen_shader_desc.emplace_back();
		strcpy(ray_gen.name_id, ray_gen_entry_point.c_str());
		ray_gen.blob = ray_gen_shader;

		ray_trace_shader_programs.ray_gen_shader = &ray_gen_shader;
		ray_trace_shader_programs.miss_shader = &miss_shader;
		ray_trace_shader_programs.closest_hit_shader = &closest_hit_shader;

		pipeline.create(device, pipeline_layout, ray_trace_shader_programs);

		// shader binding table
		sbt.create(device, pipeline, ray_trace_shader_programs);

		// copy vertices e indices from scene
		scene.max_size = 6 * 1024 * 1024;
		scene.enable_rt_features(true);
		size_t tmp_buffer_offset = 0;
		size_t rt_buffer_index = 0;
		RhiSharedBuffer tmp_buffer;
		tmp_buffer.create(device, scene.max_size);
		
		command_queue.sync_exec([&](RhiCommandQueueBufferList& list) {

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
							indices_ptr.push_back(*mesh->index_buffer);
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
				
			list.add_command_buffer(command_buffer);
		});

		// create camera and setup transform
		camera_transforms.create(device, sizeof(CameraCBRT));
		Eigen::Vector3f min, max;
		min.x() = scene.bb_min[0];
		min.y() = scene.bb_min[1];
		min.z() = scene.bb_min[2];
		max.x() = scene.bb_max[0];
		max.y() = scene.bb_max[1];
		max.z() = scene.bb_max[2];

		Eigen::Vector3f center = (min + max) * 0.5f;

		Eigen::Vector3f size =	max - min;

		float max_dimension =
			std::max({
				size.x(),
				size.y(),
				size.z()
				});

		camera_matrices.camera_pos =
			center  + Eigen::Vector3f(
				0.0f,
				0.0f,
				max_dimension * 1.0f);
		camera_matrices.camera_forward = (center - camera_matrices.camera_pos).normalized();
		camera_matrices.camera_right =
			Vec3(1.0f, 0.0f, 0.0f);
		camera_matrices.camera_up =
			Vec3(0.0f, 1.0f, 0.0f);
		camera_matrices.tanHalfFov =
			0.76f;
		camera_matrices.aspect = image_aspect;

		// view render_target (GPU read write)
		render_target_view = render_target.new_rw_view(device);

		// view camera (constant buffer)
		camera_transform_view = camera_transforms.new_constant_buffer_view(device);		// cb reg 0

		// create render pass
		rt_render_pass.create(device);

		// add views for transform buffers for shader visibility
		// creation order is related with shader constant buffer registers ids
		camera_transform_view = camera_transforms.new_constant_buffer_view(device);		// cb reg 0

		// map constant buffers
		camera_constant_buffer_map = std::make_unique<RhiSharedBufferMap>(camera_transforms.map(0, sizeof(CameraCB)));

		});

	unit_test_callbacks.on_draw = ([&](RhiUnitTest& unit_test) {

		rt_render_pass.set_pipeline(unit_test.ray_trace_pipeline);

		float dt = get_delta_time();
		//rotation_matrix = rotate_triangle(dt);
	//	geometry_buffer.update(unit_test.device, unit_test.command_buffer, geometry_instances);

		// upload shaders constants
		memcpy(camera_constant_buffer_map->get_data(), &camera_matrices, sizeof(CameraCB));

		rt_render_pass.set_render_target(render_target_view);

		rt_render_pass.render(unit_test.command_buffer, [&](RhiCommandBuffer& command_buffer) {

			command_buffer.ray_trace(render_target, sbt);
			});

		if (callbacks)
			callbacks->on_draw(unit_test);

		unit_test.swap_chain.blit(unit_test.command_buffer, render_target);

		print_fps();
		});

	unit_test_callbacks.on_end = ([&](RhiUnitTest& unit_test) {

		if (callbacks)
			callbacks->on_end(unit_test);

		camera_transforms.unmap(*camera_constant_buffer_map);
		});

	test_create_swap_chain_obj(&unit_test_callbacks);
}