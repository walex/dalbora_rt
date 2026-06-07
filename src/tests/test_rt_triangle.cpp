#include "test_api.hpp"

float aspect = 800.0f / 600.0f;
float x = 0.5f;

static struct Vertex
{
	float x, y, z;
};

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

void test_rt_triangle(fptr_test_on_init on_init,
	fptr_test_on_draw on_draw,
	fptr_test_on_end on_end,
	fptr_test_on_layout on_layout,
	fptr_test_on_configure_device on_configure_device)
{

	std::unique_ptr<RHI_BUFFER> vertex_buffer;
	std::unique_ptr<RHI_BUFFER> index_buffer;
	std::unique_ptr<RHI_PIPELINE_LAYOUT> pipeline_layout;
	std::shared_ptr<RHI_TEXTURE_2D> render_target;
	std::shared_ptr<RHI_VIEW> render_target_view;
	std::unique_ptr<RHI_RENDER_PASS> rt_render_pass;
	std::unique_ptr<RHI_RT_BVH> bvh;
	std::unique_ptr<RHI_BUFFER> bvh_instances;
	std::unique_ptr<RHI_VIEW> bvh_instances_view;
	std::unique_ptr<RHI_SBT_TABLE> sbt;
	std::unique_ptr<RHI_RT_PIPELINE> pipeline;
	std::unique_ptr<RHI_BUFFER> shared_camera_constant_buffer;
	std::unique_ptr<RHI_VIEW> camera_constant_buffer_view;
	Eigen::Matrix4f rotation_matrix = Eigen::Matrix4f::Identity();

	RHI_VOID_PTR camera_constant_buffer_ptr;

	const std::filesystem::path shaders_folder(R"(C:\Users\wadrw\Documents\develop\projects\personal\rtx\dalbora_rt\src\tests)");

	RHI_SWAP_CHAIN* swap_chain_ptr;

	CameraCBRT camera;
	camera.camera_pos =
		Vec3(0.0f, 0.0f, -3.0f);

	camera.camera_forward =
		Vec3(0.0f, 0.0f, 1.0f);

	camera.camera_right =
		Vec3(1.0f, 0.0f, 0.0f);

	camera.camera_up =
		Vec3(0.0f, 1.0f, 0.0f);

	camera.tanHalfFov =
		0.7002075f;

	camera.aspect = aspect;

	test_swap_chain([&](RHI_DEVICE& dev, RHI_COMMAND_QUEUE& command_queue,
		RHI_COMMAND_BUFFER& command_buffer, RHI_SWAP_CHAIN& swap_chain)
		{
			swap_chain_ptr = &swap_chain;

			RHI_TEXTURE_2D_DESC tx_desc;
			tx_desc.device = &dev;
			tx_desc.memory_type = buffer_memory_type_gpu_only;
			tx_desc.type = buffer_type_image_2d;
			tx_desc.width = 800;
			tx_desc.height = 600;
			tx_desc.format = resource_format_R8G8B8A8_norm;
			tx_desc.is_render_target = true;
			tx_desc.mips = 1;
			tx_desc.flags = resource_flags_shader_read_write;
			render_target.reset(rhi_texture_2d_create(&tx_desc));
			
			// compile shaders
			rhi_shaders_compiler_set_folder(shaders_folder.string().c_str());
			std::string rt_file = "simple_rt.hlsl";
			
			auto ray_gen_shader = rhi_shaders_compiler_compile(rt_file.c_str(), "RayGen", "lib_6_6");
			auto miss_shader = rhi_shaders_compiler_compile(rt_file.c_str(), "Miss", "lib_6_6");
			auto closest_hit_shader = rhi_shaders_compiler_compile(rt_file.c_str(), "ClosestHit", "lib_6_6");

			RHI_PIPELINE_LAYOUT_DESC pl_desc;
			pl_desc.device = &dev;

			// descriptors ( order mathers )

			// 1 - GPU read only (Scene BVH)
			RHI_SHADER_DESCRIPTOR_DESC& s_desc = pl_desc.descriptors[pl_desc.descriptor_count++];
			s_desc.resource_type = resource_type_shader;
			s_desc.shader_register_start = 0;
			s_desc.shader_register_max = 100; // max registers for this type, can be used for any resource of this type,
												// just need to specify the correct register in the shader

			// 2 - GPU read write (Render Target)
			RHI_SHADER_DESCRIPTOR_DESC& o_desc = pl_desc.descriptors[pl_desc.descriptor_count++];
			o_desc.resource_type = resource_type_generic_rw_buffer;
			o_desc.shader_register_start = 0;
			o_desc.shader_register_max = 100; // max registers for this type, can be used for any resource of this type,
												// just need to specify the correct register in the shader

			// 3 - Constant buffer (Camera)
			RHI_SHADER_DESCRIPTOR_DESC& c_desc = pl_desc.descriptors[pl_desc.descriptor_count++];
			c_desc.resource_type = resource_type_constant_buffer;
			c_desc.shader_register_start = 0;
			c_desc.shader_register_max = 100; // max registers for this type, can be used for any resource of this type,
												// just need to specify the correct register in the shader

			pipeline_layout.reset(rhi_pipeline_layout_create(&pl_desc));

			// rt pipeline config
			RHI_RT_PIPELINE_DESC p_desc;
			p_desc.device = &dev;
			p_desc.layout = pipeline_layout.get();

			const char ray_gen_id[16] = "RayGen";
			const char miss_id[16] = "Miss";
			const char hit_group_id_0[16] = "HG_1";
			const char group_id_0_closest_hit_id[16] = "ClosestHit";

			auto& hg_1 = p_desc.hit_groups[p_desc.hit_group_count++];
			strcpy_s(hg_1.name_id, hit_group_id_0);
			hg_1.closest_hit.blob = closest_hit_shader;
			strcpy_s(hg_1.closest_hit.name_id, group_id_0_closest_hit_id);

			auto& miss_1 = p_desc.miss_shaders[p_desc.miss_shader_count++];
			miss_1.blob = miss_shader;
			strcpy_s(miss_1.name_id, miss_id);

			auto& ray_gen = p_desc.ray_gen[p_desc.ray_gen_count++];
			strcpy(ray_gen.name_id, ray_gen_id);
			ray_gen.blob = ray_gen_shader;

			pipeline.reset(rhi_rt_pipeline_create(&p_desc));

			// sbt			
			RHI_RT_SBT_DESC sbt_desc;
			sbt_desc.ray_gen_ids[0] = ray_gen_id;
			sbt_desc.ray_gen_count = 1;
			sbt_desc.miss_ids[0] = miss_id;
			sbt_desc.miss_shader_count = 1;
			sbt_desc.hit_group_ids[0] = hit_group_id_0;
			sbt_desc.hit_group_count = 1;
			sbt.reset(rhi_rt_pipeline_create_sbt(&dev, &sbt_desc, pipeline.get()));

			// create geometry buffers
			RHI_VERTEX_BUFFER_DESC vb_desc;
			vb_desc.device = &dev;
			vb_desc.count = vertex_count;
			vb_desc.length = vertex_count * sizeof(Vertex);
			vb_desc.stride = sizeof(Vertex);
			vb_desc.format = resource_format_float3;
			vb_desc.memory_type = buffer_memory_type_gpu_only;
			vb_desc.type = buffer_type_raw;
			vertex_buffer.reset(rhi_buffers_create_vertices(&vb_desc));

			RHI_INDEX_BUFFER_DESC ib_desc;
			ib_desc.device = &dev;
			ib_desc.count = index_count;
			ib_desc.length = sizeof(uint16_t) * index_count;
			ib_desc.stride = sizeof(uint16_t);
			ib_desc.format = resource_format_uint16;
			ib_desc.memory_type = buffer_memory_type_gpu_only;
			ib_desc.type = buffer_type_raw;
			index_buffer.reset(rhi_buffers_create_indices(&ib_desc));

			// create shared memory for camera transforms
			RHI_BUFFER_DESC shared_camera_buffer_desc;
			shared_camera_buffer_desc.device = &dev;
			shared_camera_buffer_desc.length = sizeof(CameraCBRT);
			shared_camera_buffer_desc.memory_type = buffer_memory_type_shared_rw;
			shared_camera_buffer_desc.type = buffer_type_raw;
			shared_camera_buffer_desc.mips = 1;
			shared_camera_constant_buffer.reset(rhi_buffers_create_constant(&shared_camera_buffer_desc));
			
			// open cb for write
			camera_constant_buffer_ptr = rhi_buffers_map_open(shared_camera_constant_buffer.get(), 0, sizeof(CameraCBRT));

			std::unique_ptr<RHI_BUFFER> shared_vertex_buffer;
			std::unique_ptr<RHI_BUFFER> shared_index_buffer;

			rhi_command_queue_execute(&command_queue, true, [&](RHI_VOID_PTR UNUSED_PARAM(native_command_queue_impl),
				std::vector<RHI_COMMAND_BUFFER*>* command_buffer_list) {

					rhi_command_buffer_record(&command_buffer, [&](RHI_VOID_PTR UNUSED_PARAM(native_command_buffer_impl)) {

						auto vertex_size = sizeof(Vertex);
						auto vertices_ptr = &vertices[0];
									
						// cpu bridge buffer uploading
						{
							RHI_BUFFER_DESC shared_buffer_desc;
							shared_buffer_desc.device = &dev;
							shared_buffer_desc.length = vb_desc.length;
							shared_buffer_desc.memory_type = buffer_memory_type_shared_rw;
							shared_buffer_desc.type = buffer_type_raw;
							shared_buffer_desc.mips = 1;
							shared_vertex_buffer.reset(rhi_buffers_create_raw(&shared_buffer_desc));
							rhi_buffers_map_write(shared_vertex_buffer.get(), vertices_ptr, 0, shared_buffer_desc.length);
							rhi_buffers_gpu_upload(&command_buffer, shared_vertex_buffer.get(), vertex_buffer.get());
						}

						// cpu bridge buffer uploading
						{
							RHI_BUFFER_DESC shared_buffer_desc;
							shared_buffer_desc.device = &dev;
							shared_buffer_desc.length = ib_desc.length;
							shared_buffer_desc.memory_type = buffer_memory_type_shared_rw;
							shared_buffer_desc.type = buffer_type_raw;
							shared_buffer_desc.mips = 1;
							shared_index_buffer.reset(rhi_buffers_create_raw(&shared_buffer_desc));
							rhi_buffers_map_write(shared_index_buffer.get(), &indices[0], 0, shared_buffer_desc.length);
							rhi_buffers_gpu_upload(&command_buffer, shared_index_buffer.get(), index_buffer.get());
						}

						RHI_RT_BVH_DESC blas_desc;
						blas_desc.device = &dev;
						blas_desc.command_buffer = &command_buffer;
						RHI_BUFFER* vb = vertex_buffer.get();
						RHI_BUFFER* ib = index_buffer.get();
						blas_desc.vertex_buffer = &vb;
						blas_desc.index_buffer = &ib;
						blas_desc.count = 1;
						bvh.reset(rhi_rt_bvh_create(&blas_desc));

						float* matrices[] = { rotation_matrix.data() };

						RHI_RT_BVH_GEOMETRY_INSTANCES_DESC tlas_desc;
						tlas_desc.device = &dev;
						tlas_desc.command_buffer = &command_buffer;
						tlas_desc.parent_bvh = bvh.get();
						tlas_desc.transforms = &matrices[0];
						tlas_desc.instance_count = 1;
						tlas_desc.read_only = false;
						bvh_instances.reset(rhi_rt_bvh_build_geometry_instances(&tlas_desc));

						

					});
					command_buffer_list->push_back(&command_buffer);
				});

			// view BVH (GPU read only)
			RHI_VIEW_DESC bvh_instances_view_desc;
			bvh_instances_view_desc.device = &dev;
			bvh_instances_view_desc.buffer = bvh_instances.get();
			bvh_instances_view_desc.type = resource_type_rt_bvh_buffer;
			bvh_instances_view_desc.slot_id = 0;
			bvh_instances_view.reset(rhi_buffers_create_view(&bvh_instances_view_desc));

			// view render_target (GPU read write)
			RHI_VIEW_DESC rt_instances_view_desc;
			rt_instances_view_desc.format = tx_desc.format;
			rt_instances_view_desc.device = &dev;
			rt_instances_view_desc.buffer = dynamic_cast<RHI_BUFFER*>(render_target.get());
			rt_instances_view_desc.type = resource_type_texture_2d_rw;
			rt_instances_view_desc.slot_id = 100;
			render_target_view.reset(rhi_buffers_create_view(&rt_instances_view_desc));

			// view camera (constant buffer)
			RHI_VIEW_DESC cb_instances_view_desc;
			cb_instances_view_desc.format = tx_desc.format;
			cb_instances_view_desc.device = &dev;
			cb_instances_view_desc.buffer = static_cast<RHI_BUFFER*>(shared_camera_constant_buffer.get());
			cb_instances_view_desc.type = resource_type_constant_buffer;
			cb_instances_view_desc.slot_id = 200;
			camera_constant_buffer_view.reset(rhi_buffers_create_view(&cb_instances_view_desc));

			// create render pass
			RHI_RENDER_PASS_DESC render_pass_desc;
			render_pass_desc.device = &dev;
			rt_render_pass.reset(rhi_render_pass_create(&render_pass_desc));
			rt_render_pass->render_target_view = render_target_view.get();
		}	
		, [&](RHI_RENDER_PASS&) {
			// before draw

			rt_render_pass->pipeline = pipeline.get();
			
		}
		, [&] (RHI_DEVICE& dev, RHI_RENDER_PASS& render_pass, RHI_COMMAND_BUFFER& command_buffer) {

			float dt = get_delta_time();
			rotation_matrix = rotate_triangle(dt);

			float* matrices[] = { rotation_matrix.data() };

			RHI_RT_BVH_GEOMETRY_INSTANCES_DESC tlas_desc;
			tlas_desc.device = &dev;
			tlas_desc.command_buffer = &command_buffer;
			tlas_desc.parent_bvh = bvh.get();
			tlas_desc.transforms = &matrices[0];
			tlas_desc.instance_count = 1;
			
			rhi_rt_bvh_update_geometry_instances(&tlas_desc, bvh_instances.get());

			// draw
			memcpy(camera_constant_buffer_ptr, &camera, sizeof(CameraCBRT));

			rhi_render_pass_execute_rt_mode(rt_render_pass.get(), &command_buffer, [&] {				
		
				rhi_command_buffer_ray_trace(
					&command_buffer,
					render_target.get(),
					sbt.get());
			});

			const RHI_VIEW* back_buffer = rhi_swap_chain_get_surface(swap_chain_ptr, INT64_MAX);
			rhi_command_buffer_copy_texture(&command_buffer, dynamic_cast<RHI_TEXTURE_2D*>(back_buffer->buffer), render_target.get());
		}
		, [&](RHI_RENDER_PASS& render_pass, RHI_SWAP_CHAIN& swap_chain, RHI_COMMAND_BUFFER& command_buffer) {

			// on_before_present
			
		},
		[&](RHI_DEVICE& UNUSED_PARAM(device))
			{
				// if (on_end)
				//	on_end(device);

				rhi_buffers_map_close(shared_camera_constant_buffer.get(), 0, sizeof(CameraCBRT));
			}
		);
}

void test_rt_triangle_obj(RhiUnitTestCallbacks* callbacks) {

	RhiRenderTarget render_target;
	RhiView render_target_view;
	RhiShaderProgram ray_gen_shader;
	RhiShaderProgram miss_shader;
	RhiShaderProgram closest_hit_shader;
	RhiGPUBuffer vertex_buffer;
	RhiGPUBuffer index_buffer;
	RhiRayTraceGeometryBuffer geometry_buffer;
	RhiSharedBuffer camera_transforms;
	RhiView camera_transform_view;
	std::vector<RhiView> bvh_instances_views;
	RhiRayTraceRenderPass rt_render_pass;
	RhiShaderBindingTable sbt;

	std::unique_ptr<RhiSharedBufferMap> camera_constant_buffer_map;
	CameraCBRT camera_matrices;
	camera_matrices.camera_pos =
		Vec3(0.0f, 0.0f, -3.0f);
	camera_matrices.camera_forward =
		Vec3(0.0f, 0.0f, 1.0f);
	camera_matrices.camera_right =
		Vec3(1.0f, 0.0f, 0.0f);
	camera_matrices.camera_up =
		Vec3(0.0f, 1.0f, 0.0f);
	camera_matrices.tanHalfFov =
		0.7002075f;
	camera_matrices.aspect = aspect;

	std::vector<float*> geometry_instances;
	Eigen::Matrix4f rotation_matrix = Eigen::Matrix4f::Identity();
	geometry_instances.push_back(rotation_matrix.data());

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

		// create camera transform buffer
		camera_transforms.create(device, sizeof(CameraCBRT));

		// copy vertices to cpu visible memory
		RhiSharedBuffer shared_vertex_buffer;
		vertex_buffer.create(device, unit_test.vertices.size(), unit_test.vertices_stride, resource_format_float3);
		shared_vertex_buffer.create(device, unit_test.vertices.size());
		auto v_map_info = shared_vertex_buffer.map(0, unit_test.vertices.size());
		memcpy(v_map_info.get_data(), unit_test.vertices.data(), v_map_info.get_length());
		shared_vertex_buffer.unmap(v_map_info);

		// copy indices to cpu visible memory
		RhiSharedBuffer shared_index_buffer;
		index_buffer.create(device, unit_test.indices.size(), unit_test.indices_stride, resource_format_uint16);
		shared_index_buffer.create(device, unit_test.indices.size());
		auto i_map_info = shared_index_buffer.map(0, unit_test.indices.size());
		memcpy(i_map_info.get_data(), unit_test.indices.data(), i_map_info.get_length());
		shared_index_buffer.unmap(i_map_info);		

		// upload vertices e indices data to gpu only memory
		command_queue.sync_exec([&](RhiCommandQueueBufferList& list) {

			command_buffer.record([&] {

				// create geometry buffer
				vertex_buffer.upload(command_buffer, shared_vertex_buffer);
				index_buffer.upload(command_buffer, shared_index_buffer);
				geometry_buffer.create(device, command_buffer, geometry_instances, vertex_buffer, &index_buffer);
			});

			list.add_command_buffer(command_buffer);
		});

		// views	
		// 
		// view BVH (GPU read only)
		bvh_instances_views = geometry_buffer.new_view(device);

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
		rotation_matrix = rotate_triangle(dt);
		geometry_buffer.update(unit_test.device, unit_test.command_buffer, geometry_instances);

		// upload shaders constants
		memcpy(camera_constant_buffer_map->get_data(), &camera_matrices, sizeof(CameraCB));
		
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
		
		camera_transforms.unmap(*camera_constant_buffer_map);
	});

	test_create_swap_chain_obj(&unit_test_callbacks);
}