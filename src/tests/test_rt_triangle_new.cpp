#include "test_api.hpp"
#include "rhi.hpp"

#ifdef TEST_RT_TRIANGLE

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
	RHI_VOID_PTR camera_constant_buffer_ptr;
	Eigen::Matrix4f rotation_matrix = Eigen::Matrix4f::Identity();

	float aspect = 800.0f / 600.0f;
	float x = 0.5f;

	struct Vertex
	{
		float x, y, z;
	};

	Vertex vertices[] =
	{
		{0.0f, x, 0.0f}, // top
		{x, -x, 0.0f},	 // right
		{-x, -x, 0.0f}	 // left
	};
	constexpr unsigned int vertex_count = sizeof(vertices) / sizeof(Vertex);

	uint16_t indices[] =
	{
		0, 1, 2 };
	constexpr unsigned int index_count = sizeof(indices) / sizeof(uint16_t);

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

	test_swap_chain([&](RHI_DEVICE& device, RHI_COMMAND_QUEUE& command_queue,
		RHI_COMMAND_BUFFER& command_buffer, RHI_SWAP_CHAIN& swap_chain) {
			std::unique_ptr<RHI_BUFFER> shared_vertex_buffer;
			std::unique_ptr<RHI_BUFFER> shared_index_buffer;
			std::unique_ptr<RHI_COMPILED_SHADER_BUFFER> ray_gen_shader;
			std::unique_ptr<RHI_COMPILED_SHADER_BUFFER> miss_shader;
			std::unique_ptr<RHI_COMPILED_SHADER_BUFFER> closest_hit_shader;

			// on_init
			swap_chain_ptr = &swap_chain;

			// render target
			RHI_TEXTURE_2D_DESC tx_desc;
			tx_desc.device = &device;
			tx_desc.memory_type = buffer_memory_type_gpu_only;
			tx_desc.type = buffer_type_image_2d;
			tx_desc.width = 800;
			tx_desc.height = 600;
			tx_desc.format = resource_format_R8G8B8A8_norm;
			tx_desc.is_render_target = true;
			tx_desc.mips = 1;
			tx_desc.flags = resource_flags_shader_read_write;
			render_target.reset(rhi_texture_2d_create(&tx_desc));

			// create render target view
			RHI_VIEW_DESC view_desc;
			view_desc.type = resource_type_render_target;
			view_desc.device = &device;
			view_desc.format = tx_desc.format;
			view_desc.buffer = dynamic_cast<RHI_BUFFER*>(render_target.get());
			render_target_view.reset(rhi_buffers_create_view(&view_desc));

			// create render pass
			RHI_RENDER_PASS_DESC render_pass_desc;
			render_pass_desc.device = &device;
			render_pass_desc.render_target_view = render_target_view.get();
			rt_render_pass.reset(rhi_render_pass_create(&render_pass_desc));

			// compile shaders
			rhi_shaders_compiler_set_folder(shaders_folder.string().c_str());
			std::string rt_file = "simple_rt.hlsl";

			ray_gen_shader.reset(rhi_shaders_compiler_compile(rt_file.c_str(), "RayGen", "lib_6_6"));
			miss_shader.reset(rhi_shaders_compiler_compile(rt_file.c_str(), "Miss", "lib_6_6"));			
			closest_hit_shader.reset(rhi_shaders_compiler_compile(rt_file.c_str(), "ClosestHit", "lib_6_6"));

			// on_layout

			RHI_PIPELINE_LAYOUT_DESC pl_desc;
			pl_desc.device = &device;

			// descriptors

			// SceneBVH
			RHI_DESCRIPTOR_DESC& s_desc = pl_desc.descriptors[pl_desc.descriptor_count++];
			s_desc.resource_type = resource_type_shader;
			s_desc.register_start = 0;
			s_desc.register_count = 1;

			// Output
			RHI_DESCRIPTOR_DESC& o_desc = pl_desc.descriptors[pl_desc.descriptor_count++];
			o_desc.resource_type = resource_type_generic_rw_buffer;
			o_desc.register_start = 0;
			o_desc.register_count = 1;

			// Camera
			RHI_DESCRIPTOR_DESC& c_desc = pl_desc.descriptors[pl_desc.descriptor_count++];
			c_desc.resource_type = resource_type_constant_buffer;
			c_desc.register_start = 0;
			c_desc.register_count = 1;

			pipeline_layout.reset(rhi_pipeline_layout_create(&pl_desc));

			RHI_RT_PIPELINE_DESC p_desc;
			p_desc.device = &device;
			p_desc.layout = pipeline_layout.get();
			
			const char ray_gen_id[16] = "RayGen";
			const char miss_id[16] = "Miss";
			const char hit_group_id_0[16] = "HG_1";
			const char group_id_0_closest_hit_id[16] = "ClosestHit";

			auto& hg_1 = p_desc.hit_groups[p_desc.hit_group_count++];
			strcpy_s(hg_1.group_id, hit_group_id_0);
			hg_1.closest_hit.blob = closest_hit_shader.get();
			strcpy_s(hg_1.closest_hit.name_id, group_id_0_closest_hit_id);

			auto& miss_1 = p_desc.miss_shaders[p_desc.miss_shader_count++];
			miss_1.blob = miss_shader.get();
			strcpy_s(miss_1.name_id, miss_id);

			auto& ran_gen = p_desc.ray_gen[p_desc.ray_gen_count++];
			strcpy(ran_gen.name_id, ray_gen_id);
			ran_gen.blob = ray_gen_shader.get();

			pipeline.reset(rhi_rt_pipeline_create(&p_desc));
			
			// sbt			
			RHI_RT_SBT_DESC sbt_desc;
			sbt_desc.ray_gen_ids[0] = ray_gen_id;
			sbt_desc.ray_gen_count = 1;
			sbt_desc.miss_ids[0] = miss_id;
			sbt_desc.miss_shader_count = 1;
			sbt_desc.hit_group_ids[0] = hit_group_id_0;
			sbt_desc.hit_group_count = 1;
			sbt.reset(rhi_rt_pipeline_create_sbt(&device, &sbt_desc, pipeline.get()));
			
			// create geometry buffers
			RHI_VERTEX_BUFFER_DESC vb_desc;
			vb_desc.device = &device;
			vb_desc.count = vertex_count;
			vb_desc.length = vertex_count * sizeof(Vertex);
			vb_desc.stride = sizeof(Vertex);
			vb_desc.format = resource_format_float3;
			vb_desc.memory_type = buffer_memory_type_gpu_only;
			vb_desc.type = buffer_type_raw;
			vertex_buffer.reset(rhi_buffers_create_vertices(&vb_desc));

			RHI_INDEX_BUFFER_DESC ib_desc;
			ib_desc.device = &device;
			ib_desc.count = index_count;
			ib_desc.length = sizeof(uint16_t) * index_count;
			ib_desc.stride = sizeof(uint16_t);
			ib_desc.format = resource_format_uint16;
			ib_desc.memory_type = buffer_memory_type_gpu_only;
			ib_desc.type = buffer_type_raw;
			index_buffer.reset(rhi_buffers_create_indices(&ib_desc));

			// create shared memory for camera transforms
			RHI_BUFFER_DESC shared_camera_buffer_desc;
			shared_camera_buffer_desc.device = &device;
			shared_camera_buffer_desc.length = sizeof(CameraCBRT);
			shared_camera_buffer_desc.memory_type = buffer_memory_type_shared_rw;
			shared_camera_buffer_desc.type = buffer_type_raw;
			shared_camera_buffer_desc.mips = 1;
			shared_camera_constant_buffer.reset(rhi_buffers_create_constant(&shared_camera_buffer_desc));
			camera_constant_buffer_ptr = rhi_buffers_map_open(shared_camera_constant_buffer.get(), 0, sizeof(CameraCBRT));

			// camera constant buffer view
			RHI_VIEW_DESC camera_cb_view_desc;
			camera_cb_view_desc.device = &device;
			camera_cb_view_desc.buffer = shared_camera_constant_buffer.get();
			camera_cb_view_desc.type = resource_type_constant_buffer;
			camera_constant_buffer_view.reset(rhi_buffers_create_view(&camera_cb_view_desc));
			
			rhi_command_queue_execute(&command_queue, true, [&](RHI_VOID_PTR UNUSED_PARAM(native_command_queue_impl),
				std::vector<RHI_COMMAND_BUFFER*>* command_buffer_list) {

				rhi_command_buffer_record(&command_buffer, [&](RHI_VOID_PTR UNUSED_PARAM(native_command_buffer_impl)) {

					auto vertex_size = sizeof(Vertex);
					auto vertices_ptr = &vertices[0];

					// cpu bridge buffer uploading
					{
						RHI_BUFFER_DESC shared_buffer_desc;
						shared_buffer_desc.device = &device;
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
						shared_buffer_desc.device = &device;
						shared_buffer_desc.length = ib_desc.length;
						shared_buffer_desc.memory_type = buffer_memory_type_shared_rw;
						shared_buffer_desc.type = buffer_type_raw;
						shared_buffer_desc.mips = 1;
						shared_index_buffer.reset(rhi_buffers_create_raw(&shared_buffer_desc));
						rhi_buffers_map_write(shared_index_buffer.get(), &indices[0], 0, shared_buffer_desc.length);
						rhi_buffers_gpu_upload(&command_buffer, shared_index_buffer.get(), index_buffer.get());
					}

					RHI_RT_BVH_DESC blas_desc;
					blas_desc.device = &device;
					blas_desc.command_buffer = &command_buffer;
					blas_desc.vertex_buffer = vertex_buffer.get();
					blas_desc.index_buffer = index_buffer.get();
					bvh.reset(rhi_rt_bvh_create(&blas_desc));
												
					RT_GEOMETRY_INSTANCES_DESC tlas_desc;
					tlas_desc.device = &device;
					tlas_desc.command_buffer = &command_buffer;
					tlas_desc.parent_bvh = bvh.get();
					tlas_desc.transforms = &rotation_matrix;
					tlas_desc.instance_count = 1;
					bvh_instances.reset(rhi_rt_bvh_build_geometry_instances(&tlas_desc));
						
				});
				command_buffer_list->push_back(&command_buffer);
			});
			// view
			RHI_VIEW_DESC bvh_instances_view_desc;
			bvh_instances_view_desc.device = &device;
			bvh_instances_view_desc.buffer = bvh_instances.get();
			bvh_instances_view_desc.type = resource_type_rt_bvh_buffer;
			bvh_instances_view.reset(rhi_buffers_create_view(&bvh_instances_view_desc));	
		}	
		, [&](RHI_RENDER_PASS&) {
			// before draw
			
			rt_render_pass->pipeline = pipeline.get();
		}
		, [&] (RHI_DEVICE& device, RHI_RENDER_PASS& render_pass, RHI_COMMAND_BUFFER& command_buffer) {
			
			float dt = get_delta_time();
			Eigen::Matrix4f rotation_matrix = rotate_triangle(dt);

			// draw
			memcpy(camera_constant_buffer_ptr, &camera, sizeof(CameraCBRT));

			rhi_render_pass_execute_rt_mode(rt_render_pass.get(), &command_buffer, [&] {
								
				//RT_GEOMETRY_INSTANCES_DESC tlas_desc;
				//tlas_desc.device = &device;
				//tlas_desc.command_buffer = &command_buffer;
				//tlas_desc.parent_bvh = bvh.get();
				//tlas_desc.transforms = &rotation_matrix;
				//tlas_desc.instance_count = 1;
				//bvh_instances.reset(rhi_rt_bvh_build_geometry_instances(&tlas_desc));

				//// view
				//rhi_buffers_update_view(&device, bvh_instances_view.get(), bvh_instances.get());

				rhi_command_buffer_ray_trace( 
					&command_buffer, 
					render_target.get(),
					bvh_instances.get(), 
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
				return;
				// if (on_end)
				//	on_end(device);

				// on end
				rhi_buffers_map_close(shared_camera_constant_buffer.get(), 0, sizeof(CameraCBRT));
			},
		[&](RHI_DEVICE_DESC& desc) {
			if (on_configure_device)
				on_configure_device(desc);
			desc.features = device_features_raytracing;
		});
}

#endif