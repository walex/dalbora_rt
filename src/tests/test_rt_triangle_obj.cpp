#include "rhi.hpp"
#include "test_api_helpers.hpp"

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

// memory descriptors
static std::unique_ptr<RhiMemoryDescriptors> buffers_memory_table;
static std::unique_ptr<RhiMemoryDescriptors> rtv_memory_table;
static std::unique_ptr<RhiMemoryDescriptors> dsv_memory_table;
static std::unique_ptr<RhiMemoryDescriptors> samplers_memory_table;

static RhiMemoryDescriptors& get_buffers_memory_descriptors() {
	return *buffers_memory_table;
}
static RhiMemoryDescriptors& get_rtv_memory_descriptors() {
	return *rtv_memory_table;
}
static RhiMemoryDescriptors& get_dsv_memory_descriptors() {
	return *dsv_memory_table;
}
static RhiMemoryDescriptors& get_samplers_memory_descriptors() {
	return *samplers_memory_table;
}

static void memory_descriptors_init(RHI_DEVICE* device) {

	RhiDevice device_obj(device);

	std::vector<size_t> slot_group_start_indices = { 5, 5, 5 };
	buffers_memory_table = std::make_unique<RhiMemoryDescriptors>(device_obj, memory_descriptor_type_buffer,
		BUFFERS_DESCRIPTORS_COUNT, slot_group_start_indices);
	samplers_memory_table = std::make_unique<RhiMemoryDescriptors>(device_obj, memory_descriptor_type_sampler,
		SAMPLER_DESCRIPTORS_COUNT);
	rtv_memory_table = std::make_unique<RhiMemoryDescriptors>(device_obj, memory_descriptor_type_rtv,
		RTV_HEAP_DESCRIPTORS_COUNT);
	dsv_memory_table = std::make_unique<RhiMemoryDescriptors>(device_obj, memory_descriptor_type_dsv,
		DSV_DESCRIPTORS_COUNT);
}

void test_rt_triangle_obj() {

	RhiRenderTarget render_target;
	RhiView render_target_view;
	RhiShaderProgramHLSL ray_gen_shader;
	RhiShaderProgramHLSL miss_shader;
	RhiShaderProgramHLSL closest_hit_shader;
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
	
	float4x4 rotation_matrix = float4x4::Identity();
	std::vector<std::vector<const float*>> instances_transforms;

	RhiWindow window;
	RHI_WINDOW_CALLBACKS window_callbacks;
	RhiDevice device;
	RhiGraphicsCommandQueue command_queue;
	RhiCommandBuffer command_buffer;
	RhiSwapChain swap_chain;
	RhiPipelineLayout pipeline_layout;
	RhiRayTracePipeline ray_trace_pipeline;
	RHI_VIEWPORT vp;

	window_callbacks.on_init = [&](RHI_WINDOW* hwindow) {

		constexpr size_t MAX_COMMAND_ALLOCATORS = 8;

		RHI_DEVICE_DESC device_desc;
		device_desc.adapter_id = -1;
		device_desc.shader_model = hlsl_shader_model_6_8;
		device_desc.app_instance = rhi_get_app_instance();
		device_desc.graphics_queue_count = 1;
		device_desc.compute_queue_count = 1;
		device_desc.copy_queue_count = 1;
		device_desc.features = device_features_raytracing;
		device.create(device_desc);

		memory_descriptors_init(device);

		RhiCommandAllocatorPool::Initialize(device, MAX_COMMAND_ALLOCATORS, queue_type_graphics);
		RhiCommandAllocatorPool::Initialize(device, MAX_COMMAND_ALLOCATORS, queue_type_compute);
		RhiCommandAllocatorPool::Initialize(device, MAX_COMMAND_ALLOCATORS, queue_type_copy);

		command_queue.create(device);
		command_buffer.create(device, command_queue);
		command_buffer.set_buffers_memory_descriptor(get_buffers_memory_descriptors());
		swap_chain.create(window, device, command_queue);
		swap_chain.create_views(device, get_rtv_memory_descriptors());
		
		vp.x = 0;
		vp.y = 0;
		vp.width = swap_chain.get_width();
		vp.height = swap_chain.get_height();
		vp.min_z = 0.0f;
		vp.max_z = 1.0f;

		constexpr size_t constant_shader_registers_count = 5;
		constexpr size_t read_only_shader_registers_count = 5;
		constexpr size_t rw_shader_registers_count = 5;

		// setup shaders
		std::filesystem::path shader_path = get_executable_folder("shaders");
		shader_path = shader_path / "simple_rt.hlsl";
		std::string ray_gen_shader_file = shader_path.string();
		std::string miss_shader_file = shader_path.string();
		std::string closest_hit_shader_file = shader_path.string();

		// save geometry buffers
		size_t vertices_size(sizeof(vertices));
		size_t indices_size(sizeof(indices));
		size_t vertices_stride = sizeof(vertices[0]);
		size_t indices_stride = sizeof(uint16_t);

		// create render target
		render_target.create(device, swap_chain.get_format(),
			swap_chain.get_width(), swap_chain.get_height());
		camera_matrices.aspect = static_cast<float>(swap_chain.get_width()) / static_cast<float>(swap_chain.get_height());

		byte_code_format bc_format = byte_code_format_input_hlsl;
		if (rhi_get_device_type() == rhi_device_type_vulkan) {
			int flag = static_cast<int>(bc_format) | static_cast<int>(byte_code_format_output_spirv);
			bc_format = static_cast<byte_code_format>(flag);
		}
		// compile shaders
		ray_gen_shader.create(ray_gen_shader_file,
			"RayGen", "lib_6_6", bc_format);
		miss_shader.create(miss_shader_file,
			"Miss", "lib_6_6", bc_format);
		closest_hit_shader.create(closest_hit_shader_file,
			"ClosestHit", "lib_6_6", bc_format);

		// add layout descriptors
		pipeline_layout.set_shader_descriptors(shader_view_type_constant_buffer, 0, constant_shader_registers_count);
		pipeline_layout.set_shader_descriptors(shader_view_type_read_only_buffer, 0, read_only_shader_registers_count);
		pipeline_layout.set_shader_descriptors(shader_view_type_rw_buffer, 0, rw_shader_registers_count);

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

		ray_trace_pipeline.create(device, pipeline_layout, ray_trace_shader_programs);

		// shader binding table
		sbt.create(device, ray_trace_pipeline, ray_trace_shader_programs);

		// create camera transform buffer
		camera_transforms.create(device, sizeof(CameraCBRT));

		// copy vertices to cpu visible memory
		RhiSharedBuffer shared_vertex_buffer;
		vertex_buffer.create(device, vertices_size, vertices_stride, resource_format_float3, resource_flags_rt_as);
		shared_vertex_buffer.create(device, vertices_size);
		{
			auto v_map_info = shared_vertex_buffer.map(0, vertices_size);
			memcpy(v_map_info.get_data(), vertices, v_map_info.get_length());
		}

		// copy indices to cpu visible memory
		RhiSharedBuffer shared_index_buffer;
		index_buffer.create(device, indices_size, indices_stride, resource_format_uint16, resource_flags_rt_as);
		shared_index_buffer.create(device, indices_size);
		{
			auto i_map_info = shared_index_buffer.map(0, indices_size);
			memcpy(i_map_info.get_data(), indices, i_map_info.get_length());
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
		geometry_instances_views = geometry_instances.new_view(device, get_buffers_memory_descriptors().next_descriptor_ptr(1));

		// view render_target (GPU read write)
		render_target_view = render_target.new_view(device, get_buffers_memory_descriptors().next_descriptor_ptr(2));

		// create render pass
		rt_render_pass.create(device);
		
		// add views for transform buffers for shader visibility
		// creation order is related with shader constant buffer registers ids
		camera_transform_view = camera_transforms.new_view(device, shader_view_type_constant_buffer, get_buffers_memory_descriptors().next_descriptor_ptr());		// cb reg 0
		
		// map constant buffers
		camera_constant_buffer_map = std::make_unique<RhiSharedBufferMap>(camera_transforms, 0, sizeof(CameraCBRT));


	};

	window_callbacks.on_idle = [&](RHI_WINDOW* UNUSED_PARAM(hwindow)) {

		rt_render_pass.set_pipeline(ray_trace_pipeline);

		float dt = get_delta_time();
		rotation_matrix = rotate_triangle(dt);
		std::vector<float*> new_t;
		new_t.push_back(reinterpret_cast<float*>(&rotation_matrix));

		// upload shaders constants
		memcpy(camera_constant_buffer_map->get_data(), &camera_matrices, sizeof(CameraCBRT));
		
		rt_render_pass.set_render_target(render_target_view);
		swap_chain.get_next_render_target();

		command_queue.sync_exec([&](RhiCommandQueueBufferList& list) {
			command_buffer.record([&] {
				geometry_instances.update(device, command_buffer,
					geometry_buffers.at(0), new_t);

				rt_render_pass.render(command_buffer, [&](RhiCommandBuffer& command_buffer) {
					command_buffer.ray_trace(render_target, sbt);
				});

				swap_chain.blit(command_buffer, render_target);
			});
			list.add_command_buffer(command_buffer);
		});

		swap_chain.present();
	};

	window_callbacks.on_end = [&](RHI_WINDOW* UNUSED_PARAM(hwindow)) {
	};

	window.create("test", 800, 600, false, window_callbacks);
	window.mainLoop();
}



//void test_rt_triangle_obj(RhiUnitTestCallbacks* callbacks) {
//
//	RhiRenderTarget render_target;
//	RhiView render_target_view;
//	RhiShaderProgramHLSL ray_gen_shader;
//	RhiShaderProgramHLSL miss_shader;
//	RhiShaderProgramHLSL closest_hit_shader;
//	RhiGPUBuffer vertex_buffer;
//	RhiGPUBuffer index_buffer;
//	std::vector<RhiRayTraceGeometryBuffer> geometry_buffers;
//	RhiRayTraceGeometrydBufferInstances geometry_instances;
//	RhiView geometry_instances_views;
//	RhiSharedBuffer camera_transforms;
//	RhiView camera_transform_view;
//	RhiRayTraceRenderPass rt_render_pass;
//	RhiShaderBindingTable sbt;
//
//	std::unique_ptr<RhiSharedBufferMap> camera_constant_buffer_map;
//	CameraCBRT camera_matrices;
//	camera_matrices.camera_pos =
//		float4(0.0f, 0.0f, -3.0f, 1.0f);
//	camera_matrices.camera_forward =
//		float4(0.0f, 0.0f, 1.0f, 0.0f);
//	camera_matrices.camera_right =
//		float4(1.0f, 0.0f, 0.0f, 0.0f);
//	camera_matrices.camera_up =
//		float4(0.0f, 1.0f, 0.0f, 0.0f);
//	camera_matrices.tanHalfFov =
//		0.7002075f;
//	camera_matrices.aspect = image_aspect;
//
//	float4x4 rotation_matrix = float4x4::Identity();
//	std::vector<std::vector<const float*>> instances_transforms;
//
//	RhiUnitTestCallbacks unit_test_callbacks;
//	unit_test_callbacks.on_init = ([&](RhiUnitTest& unit_test) {
//
//		RhiDevice& device = unit_test.device;
//		RhiGraphicsCommandQueue& command_queue = unit_test.command_queue;
//		RhiCommandBuffer& command_buffer = unit_test.command_buffer;
//		RhiSwapChain& swap_chain = unit_test.swap_chain;
//		RhiPipelineLayout& pipeline_layout = unit_test.pipeline_layout;
//		RhiRayTracePipeline& pipeline = unit_test.ray_trace_pipeline;
//		
//		// setup shaders
//		std::filesystem::path shader_path = get_executable_folder("shaders");
//		shader_path = shader_path / "simple_rt.hlsl";
//		unit_test.ray_gen_shader_file = shader_path.string();
//		unit_test.miss_shader_file = shader_path.string();
//		unit_test.closest_hit_shader_file = shader_path.string();
//
//		// save geometry buffers
//		unit_test.vertices.resize(sizeof(vertices));
//		memcpy(unit_test.vertices.data(), &vertices[0], sizeof(vertices));
//		unit_test.vertices_stride = sizeof(vertices[0]);
//		unit_test.indices.resize(sizeof(indices));
//		memcpy(unit_test.indices.data(), &indices[0], sizeof(indices));
//		unit_test.indices_stride = sizeof(uint16_t);
//
//		// create render target
//		render_target.create(device, swap_chain.get_format(),
//			swap_chain.get_width(), swap_chain.get_height());
//		camera_matrices.aspect = static_cast<float>(swap_chain.get_width()) / static_cast<float>(swap_chain.get_height());
//
//		if (callbacks)
//			callbacks->on_init(unit_test);
//
//		byte_code_format bc_format = byte_code_format_input_hlsl;
//		if (rhi_get_device_type() == rhi_device_type_vulkan) {
//			int flag = static_cast<int>(bc_format) | static_cast<int>(byte_code_format_output_spirv);
//			bc_format = static_cast<byte_code_format>(flag);
//		}
//		// compile shaders
//		ray_gen_shader.create(unit_test.ray_gen_shader_file,
//			"RayGen", "lib_6_6", bc_format);
//		miss_shader.create(unit_test.miss_shader_file,
//			"Miss", "lib_6_6", bc_format);
//		closest_hit_shader.create(unit_test.closest_hit_shader_file,
//			"ClosestHit", "lib_6_6", bc_format);
//
//		// add layout descriptors
//		pipeline_layout.set_shader_descriptors(shader_view_type_constant_buffer, 0, unit_test.constant_shader_registers_count);
//		pipeline_layout.set_shader_descriptors(shader_view_type_read_only_buffer, 0, unit_test.read_only_shader_registers_count);
//		pipeline_layout.set_shader_descriptors(shader_view_type_rw_buffer, 0, unit_test.rw_shader_registers_count);
//
//		// create pipeline layout
//		pipeline_layout.create(device, primitive_topology_triangle, swap_chain.get_format(), resource_format_d24_norm_s8_uint);
//
//		// rt pipeline config
//		std::string ray_gen_entry_point = "RayGen";
//		std::string miss_entry_point = "Miss";
//		std::string closest_hit_entry_point = "ClosestHit";
//
//		// config ray trace shader
//		RhiRayTracePipelineShaderPrograms ray_trace_shader_programs;
//		std::vector<RHI_RT_HIT_GROUP_DESC>& hit_groups_desc = ray_trace_shader_programs.hit_groups_desc;
//		auto& hg = hit_groups_desc.emplace_back();
//		strcpy_s(hg.name_id, "HG_1");
//		hg.closest_hit.blob = closest_hit_shader;
//		strcpy_s(hg.closest_hit.name_id, closest_hit_entry_point.c_str());		
//
//		std::vector<RHI_RT_SHADER_UNIT_DESC>& miss_shader_desc = ray_trace_shader_programs.miss_shaders_desc;
//		auto& miss_1 = miss_shader_desc.emplace_back();
//		miss_1.blob = miss_shader;
//		strcpy_s(miss_1.name_id, miss_entry_point.c_str());
//		
//		std::vector<RHI_RT_SHADER_UNIT_DESC>& ray_gen_shader_desc = ray_trace_shader_programs.ray_gen_shaders_desc;
//		auto& ray_gen = ray_gen_shader_desc.emplace_back();
//		strcpy_s(ray_gen.name_id, ray_gen_entry_point.c_str());
//		ray_gen.blob = ray_gen_shader;
//
//		ray_trace_shader_programs.ray_gen_shader = &ray_gen_shader;
//		ray_trace_shader_programs.miss_shader = &miss_shader;
//		ray_trace_shader_programs.closest_hit_shader = &closest_hit_shader;
//
//		pipeline.create(device, pipeline_layout, ray_trace_shader_programs);
//
//		// shader binding table
//		sbt.create(device, pipeline, ray_trace_shader_programs);
//
//		// create camera transform buffer
//		camera_transforms.create(device, sizeof(CameraCBRT));
//
//		// copy vertices to cpu visible memory
//		RhiSharedBuffer shared_vertex_buffer;
//		vertex_buffer.create(device, unit_test.vertices.size(), unit_test.vertices_stride, resource_format_float3, resource_flags_rt_as);
//		shared_vertex_buffer.create(device, unit_test.vertices.size());
//		{
//			auto v_map_info = shared_vertex_buffer.map(0, unit_test.vertices.size());
//			memcpy(v_map_info.get_data(), unit_test.vertices.data(), v_map_info.get_length());
//		}
//
//		// copy indices to cpu visible memory
//		RhiSharedBuffer shared_index_buffer;
//		index_buffer.create(device, unit_test.indices.size(), unit_test.indices_stride, resource_format_uint16, resource_flags_rt_as);
//		shared_index_buffer.create(device, unit_test.indices.size());
//		{
//			auto i_map_info = shared_index_buffer.map(0, unit_test.indices.size());
//			memcpy(i_map_info.get_data(), unit_test.indices.data(), i_map_info.get_length());
//		}
//		// upload vertices e indices data to gpu only memory
//		command_queue.sync_exec([&](RhiCommandQueueBufferList& list) {
//
//			command_buffer.record([&] {
//
//				// create geometry buffer
//				vertex_buffer.upload(command_buffer, shared_vertex_buffer);
//				index_buffer.upload(command_buffer, shared_index_buffer);
//				auto& geometry_buffer = geometry_buffers.emplace_back();
//				auto& transforms = instances_transforms.emplace_back();
//				transforms.push_back(reinterpret_cast<float*>(&rotation_matrix));
//				geometry_buffer.create(device, command_buffer, vertex_buffer, &index_buffer);
//				geometry_instances.create(device, command_buffer, geometry_buffers, instances_transforms);
//				
//			});
//
//			list.add_command_buffer(command_buffer);
//		});
//
//		// views	
//		// 
//		// view BVH (GPU read only)
//		geometry_instances_views = geometry_instances.new_view(device, get_buffers_memory_table().next_descriptor_ptr(1));
//
//		// view render_target (GPU read write)
//		render_target_view = render_target.new_view(device, get_buffers_memory_table().next_descriptor_ptr(2));
//
//		// create render pass
//		rt_render_pass.create(device);
//		
//		// add views for transform buffers for shader visibility
//		// creation order is related with shader constant buffer registers ids
//		camera_transform_view = camera_transforms.new_view(device, shader_view_type_constant_buffer, get_buffers_memory_table().next_descriptor_ptr());		// cb reg 0
//		
//		// map constant buffers
//		camera_constant_buffer_map = std::make_unique<RhiSharedBufferMap>(camera_transforms, 0, sizeof(CameraCBRT));
//		
//	});
//
//	unit_test_callbacks.on_device_config = ([&](RHI_DEVICE_DESC& device_desc) {
//		
//		device_desc.features |= device_features_raytracing;
//		device_desc.shader_model = hlsl_shader_model_6_8;
//
//		//device_desc.shader_resources_desc.read_only_buffer_shader_registers_count = read_only_shader_registers_count;
//		//device_desc.shader_resources_desc.rw_buffer_shader_registers_count = rw_shader_registers_count;
//		//device_desc.shader_resources_desc.constant_buffer_shader_registers_count = constant_shader_registers_count;
//
//	});
//
//	unit_test_callbacks.on_memory_descriptor_config = ([&](size_t& constant_shader_registers_count,
//		size_t& read_only_shader_registers_count, size_t& rw_shader_registers_count) {
//
//			constant_shader_registers_count = 1;
//			read_only_shader_registers_count = 1;
//			rw_shader_registers_count = 1;
//			if (callbacks)
//				return callbacks->on_memory_descriptor_config(constant_shader_registers_count, read_only_shader_registers_count,
//					rw_shader_registers_count);
//			return true;
//		});
//
//	unit_test_callbacks.on_draw = ([&](RhiUnitTest& unit_test) {
//
//		rt_render_pass.set_pipeline(unit_test.ray_trace_pipeline);
//
//		float dt = get_delta_time();
//		rotation_matrix = rotate_triangle(dt);
//		std::vector<float*> new_t;
//		new_t.push_back(reinterpret_cast<float*>(&rotation_matrix));
//
//		geometry_instances.update(unit_test.device, unit_test.command_buffer, 
//			geometry_buffers.at(0), new_t);
//
//		// upload shaders constants
//		memcpy(camera_constant_buffer_map->get_data(), &camera_matrices, sizeof(CameraCBRT));
//		
//		rt_render_pass.set_render_target(render_target_view);
//
//		rt_render_pass.render(unit_test.command_buffer, [&](RhiCommandBuffer& command_buffer) {
//
//			command_buffer.ray_trace(render_target, sbt);
//		});
//
//		if (callbacks)
//			callbacks->on_draw(unit_test);
//
//		unit_test.swap_chain.blit(unit_test.command_buffer, render_target);		
//	});
//
//	unit_test_callbacks.on_end = ([&](RhiUnitTest& unit_test) {
//
//		if (callbacks)
//			callbacks->on_end(unit_test);
//		
//	});
//
//	test_create_swap_chain_obj(&unit_test_callbacks);
//}