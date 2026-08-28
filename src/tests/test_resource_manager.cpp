#include "test_api.hpp"
#include "ResourceManager.hpp"

static float image_aspect = 800.0f / 600.0f;
static float x = 0.5f;

static Vertex vertices[] =
{
	{0.0f, x, 0.0f}, // top
	{x, -x, 0.0f},	 // right
	{-x, -x, 0.0f}	 // left
};
static constexpr unsigned int vertex_count = sizeof(vertices) / sizeof(Vertex);

static uint16_t indices[] =
{
	0, 1, 2 
};
static constexpr unsigned int index_count = sizeof(indices) / sizeof(uint16_t);

void test_rt_triangle_rm_obj(RhiUnitTestCallbacks* callbacks) {

	RhiRenderTarget render_target;
	RhiView render_target_view;
	RhiShaderProgram ray_gen_shader;
	RhiShaderProgram miss_shader;
	RhiShaderProgram closest_hit_shader;
	RhiGPUBuffer vertex_buffer;
	RhiGPUBuffer index_buffer;
	std::vector<RhiRayTraceGeometryBuffer> geometry_buffers;
	RhiRayTraceGeometrydBufferInstances geometry_instances;
	RhiView geometry_instances_views;
	RhiSharedBuffer camera_transforms;
	RhiView camera_transform_view;
	RhiRayTraceRenderPass rt_render_pass;
	RhiShaderBindingTable sbt;

	std::unique_ptr<RhiSharedBufferMap> camera_constant_buffer_map;
	CameraCBRT camera_matrices;
	camera_matrices.camera_pos =
		float4(0.0f, 0.0f, -3.0f, 1.0f);
	camera_matrices.camera_forward =
		float4(0.0f, 0.0f, 1.0f, 0.0f);
	camera_matrices.camera_right =
		float4(1.0f, 0.0f, 0.0f, 0.0f);
	camera_matrices.camera_up =
		float4(0.0f, 1.0f, 0.0f, 0.0f);
	camera_matrices.tanHalfFov =
		0.7002075f;
	camera_matrices.aspect = image_aspect;

	constexpr size_t read_only_shader_registers_count = 800;
	constexpr size_t rw_shader_registers_count = 1;
	constexpr size_t constant_shader_registers_count = 1;

	float4x4 rotation_matrix = float4x4::Identity();
	std::vector<std::vector<const float*>> instances_transforms;

	std::unique_ptr<ResourceManager> resource_manager;

	RhiUnitTestCallbacks unit_test_callbacks;
	unit_test_callbacks.on_init = ([&](RhiUnitTest& unit_test) {

		RhiWindow& window = unit_test.window;
		RhiDevice& device = unit_test.device;
		RhiGraphicsCommandQueue& command_queue = unit_test.command_queue;
		RhiCommandBuffer& command_buffer = unit_test.command_buffer;
		RhiSwapChain& swap_chain = unit_test.swap_chain;
		RhiPipelineLayout& pipeline_layout = unit_test.pipeline_layout;
		RhiRayTracePipeline& pipeline = unit_test.ray_trace_pipeline;
		
		const std::vector<size_t> space_slots = {
			1000, 1000
		};
		resource_manager = std::make_unique<ResourceManager>(device, space_slots);

		// setup shaders
		std::filesystem::path shader_path = get_executable_folder("shaders");
		shader_path = shader_path / "simple_rt.hlsl";
		unit_test.ray_gen_shader_file = shader_path.string();
		unit_test.miss_shader_file = shader_path.string();
		unit_test.closest_hit_shader_file = shader_path.string();

		// save geometry buffers
		unit_test.vertices.resize(sizeof(vertices));
		memcpy(unit_test.vertices.data(), &vertices[0], sizeof(vertices));
		unit_test.vertices_stride = sizeof(vertices[0]);
		unit_test.indices.resize(sizeof(indices));
		memcpy(unit_test.indices.data(), &indices[0], sizeof(indices));
		unit_test.indices_stride = sizeof(uint16_t);

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
		pipeline_layout.add_read_only_buffer_descriptors(0, read_only_shader_registers_count);

		// 2 - GPU read write (Render Target)
		pipeline_layout.add_rw_buffer_descriptors(0, rw_shader_registers_count);

		// 3 - Constant buffer (Camera)
		pipeline_layout.add_constants_buffer_descriptors(0, constant_shader_registers_count);		

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
		strcpy_s(ray_gen.name_id, ray_gen_entry_point.c_str());
		ray_gen.blob = ray_gen_shader;

		ray_trace_shader_programs.ray_gen_shader = &ray_gen_shader;
		ray_trace_shader_programs.miss_shader = &miss_shader;
		ray_trace_shader_programs.closest_hit_shader = &closest_hit_shader;

		pipeline.create(device, pipeline_layout, ray_trace_shader_programs);

		// shader binding table
		sbt.create(device, pipeline, ray_trace_shader_programs);

		// create camera transform buffer
		camera_transforms.create(device, sizeof(CameraCBRT));

		// copy vertices to cpu visible memory
		RhiSharedBuffer shared_vertex_buffer;
		vertex_buffer.create(device, unit_test.vertices.size(), unit_test.vertices_stride, resource_format_float3);
		shared_vertex_buffer.create(device, unit_test.vertices.size());
		{
			auto v_map_info = shared_vertex_buffer.map(0, unit_test.vertices.size());
			memcpy(v_map_info.get_data(), unit_test.vertices.data(), v_map_info.get_length());
		}

		// copy indices to cpu visible memory
		RhiSharedBuffer shared_index_buffer;
		index_buffer.create(device, unit_test.indices.size(), unit_test.indices_stride, resource_format_uint16);
		shared_index_buffer.create(device, unit_test.indices.size());
		{
			auto i_map_info = shared_index_buffer.map(0, unit_test.indices.size());
			memcpy(i_map_info.get_data(), unit_test.indices.data(), i_map_info.get_length());
		}
		// upload vertices e indices data to gpu only memory
		command_queue.sync_exec([&](RhiCommandQueueBufferList& list) {

			command_buffer.record([&] {

				// create geometry buffer
				vertex_buffer.upload(command_buffer, shared_vertex_buffer);
				index_buffer.upload(command_buffer, shared_index_buffer);
				auto& geometry_buffer = geometry_buffers.emplace_back();
				auto& transforms = instances_transforms.emplace_back();
				transforms.push_back(reinterpret_cast<float*>(&rotation_matrix));
				geometry_buffer.create(device, command_buffer, vertex_buffer, &index_buffer);
				geometry_instances.create(device, command_buffer, geometry_buffers, instances_transforms);
				
			});

			list.add_command_buffer(command_buffer);
		});

		// views	
		// 
		// view BVH (GPU read only)
		//geometry_instances_views = geometry_instances.new_view(device);
		geometry_instances_views = resource_manager->new_resource_view(device, geometry_instances);

		// view render_target (GPU read write)
		//render_target_view = render_target.new_rw_view(device);
		//render_target_view = resource_manager->new_resource_view(device, render_target);

		// create render pass
		rt_render_pass.create(device);
		
		// add views for transform buffers for shader visibility
		// creation order is related with shader constant buffer registers ids
		camera_transform_view = camera_transforms.new_constant_buffer_view(device);		// cb reg 0
		
		// map constant buffers
		camera_constant_buffer_map = std::make_unique<RhiSharedBufferMap>(camera_transforms, 0, sizeof(CameraCBRT));
		
	});

	unit_test_callbacks.on_device_config = ([&](RHI_DEVICE_DESC& device_desc) {
		
		device_desc.shader_resources_desc.read_only_buffer_shader_registers_count = read_only_shader_registers_count;
		device_desc.shader_resources_desc.rw_buffer_shader_registers_count = rw_shader_registers_count;
		device_desc.shader_resources_desc.constant_buffer_shader_registers_count = constant_shader_registers_count;

	});

	unit_test_callbacks.on_draw = ([&](RhiUnitTest& unit_test) {

		rt_render_pass.set_pipeline(unit_test.ray_trace_pipeline);

		float dt = get_delta_time();
		rotation_matrix = rotate_triangle(dt);
		std::vector<float*> new_t;
		new_t.push_back(reinterpret_cast<float*>(&rotation_matrix));

		geometry_instances.update(unit_test.device, unit_test.command_buffer, 
			geometry_buffers.at(0), new_t);

		// upload shaders constants
		memcpy(camera_constant_buffer_map->get_data(), &camera_matrices, sizeof(CameraCBRT));
		
		rt_render_pass.set_render_target(render_target_view);

		rt_render_pass.render(unit_test.command_buffer, [&](RhiCommandBuffer& command_buffer) {

			command_buffer.ray_trace(render_target, sbt);
		});

		if (callbacks)
			callbacks->on_draw(unit_test);

		unit_test.swap_chain.blit(unit_test.command_buffer, render_target);		
	});

	unit_test_callbacks.on_end = ([&](RhiUnitTest& unit_test) {

		if (callbacks)
			callbacks->on_end(unit_test);
		
	});

	test_create_swap_chain_obj(&unit_test_callbacks);
}