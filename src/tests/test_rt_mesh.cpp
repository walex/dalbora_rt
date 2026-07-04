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
	RayTraceScene scene;

	std::unique_ptr<RhiSharedBufferMap> camera_constant_buffer_map;
	CameraCBRT camera_matrices;

	RhiUnitTestCallbacks unit_test_callbacks;
	unit_test_callbacks.on_device_config = ([](__int64& feature_flags) {
		
		feature_flags |= device_features_raytracing;
	});
	unit_test_callbacks.on_init = ([&](RhiUnitTest& unit_test) {

		RhiWindow& window = unit_test.window;
		RhiDevice& device = unit_test.device;
		RhiGraphicsCommandQueue& command_queue = unit_test.command_queue;
		RhiCommandBuffer& command_buffer = unit_test.command_buffer;
		RhiSwapChain& swap_chain = unit_test.swap_chain;
		RhiPipelineLayout& pipeline_layout = unit_test.pipeline_layout;
		RhiRayTracePipeline& pipeline = unit_test.ray_trace_pipeline;

		// setup shaders
		std::filesystem::path shader_path = get_executable_folder("shaders");
		shader_path  = shader_path / "simple_rt.hlsl";
		unit_test.ray_gen_shader_file = shader_path.string();
		unit_test.miss_shader_file = shader_path.string();
		unit_test.closest_hit_shader_file = shader_path.string();
	
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

		// load scene from file
		std::filesystem::path model_3d_folder = get_executable_folder("test_3d_models");
		std::string model_3d_file = (model_3d_folder / "scene.gltf").string();
		if (std::filesystem::exists(model_3d_file) == false) {
			throw std::exception("3d model file deos not exists");
		}
		scene.set_max_size(6 * 1024 * 1024);
		scene.load(model_3d_file,
			device,	command_queue);

		// create camera and setup transform
		camera_transforms.create(device, sizeof(CameraCBRT));
		float3 bb_min = scene.get_bb_min();
		float3 bb_max = scene.get_bb_max();

		float4 center = float4((bb_min + bb_max) * 0.5f, 1.0f);

		float3 size =	bb_max - bb_min;

		float max_dimension =
			std::max({
				size.x,
				size.y,
				size.z
				});

		camera_matrices.camera_pos =
			center  + float4(
				0.0f,
				0.0f,
				max_dimension * 1.0f,
				1.0f);
		
		camera_matrices.camera_forward = (center - camera_matrices.camera_pos);
		camera_matrices.camera_forward.xyz = normalize(camera_matrices.camera_forward.xyz);
		camera_matrices.camera_right =
			float4(1.0f, 0.0f, 0.0f, 0.0f);
		camera_matrices.camera_up =
			float4(0.0f, 1.0f, 0.0f, 0.0f);
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