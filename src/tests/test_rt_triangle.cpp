#include "test_api.hpp"
#include "rhi.hpp"

void test_rt_triangle(fptr_test_on_init on_init,
	fptr_test_on_draw on_draw,
	fptr_test_on_end on_end,
	fptr_test_on_layout on_layout)
{

	std::unique_ptr<RHI_BUFFER> vertex_buffer;
	std::unique_ptr<RHI_BUFFER> index_buffer;
	std::unique_ptr<RHI_PIPELINE_LAYOUT> pipeline_layout;
	std::shared_ptr<RHI_TEXTURE_2D> render_target;
	std::unique_ptr<RHI_RENDER_PASS> rt_render_pass;
	std::unique_ptr<RHI_RT_BVH> bvh;
	std::unique_ptr<RHI_BUFFER> bvh_instances;
	std::unique_ptr<RHI_BUFFER> sbt;
	std::unique_ptr<RHI_RT_PIPELINE> pipeline;
	std::unique_ptr<RHI_CONSTANT_BUFFER> shared_camera_constant_buffer;
	RHI_VOID_PTR camera_constant_buffer_ptr;

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
		RHI_COMMAND_BUFFER& command_buffer, RHI_SWAP_CHAIN& swap_chain)
		{
			// on_init
			swap_chain_ptr = &swap_chain;

			// render target
			RHI_TEXTURE_2D_DESC tx_desc(device);
			auto back_buffer = swap_chain.get_render_target(0);
			tx_desc.memory_type = buffer_memory_type_gpu_only;
			tx_desc.type = buffer_type_image_2d;
			tx_desc.width = back_buffer->get_width();
			tx_desc.height = back_buffer->get_height();
			tx_desc.default_state = resource_state_rt_render_target;
			tx_desc.format = resource_format_R8G8B8A8_norm;
			tx_desc.resource_slot = 1;
			render_target = rhi_texture_2d_create(tx_desc);

			// create render pass
			RHI_RENDER_PASS_DESC render_pass_desc(device, render_target);
			render_pass_desc.synchronized = true;
			rt_render_pass = rhi_render_pass_create(render_pass_desc);

			// compile shaders
			rhi_shaders_compiler_set_folder(shaders_folder.string().c_str());
			std::string rt_file = "simple_rt.hlsl";
			
			auto ray_gen_shader = rhi_shaders_compiler_compile(rt_file.c_str(), "RayGen", "lib_6_6");
			auto miss_shader = rhi_shaders_compiler_compile(rt_file.c_str(), "Miss", "lib_6_6");
			auto closest_hit_shader = rhi_shaders_compiler_compile(rt_file.c_str(), "ClosestHit", "lib_6_6");

			// on_layout

			RHI_PIPELINE_LAYOUT_DESC pl_desc(device);

			// descriptors
			std::vector<RHI_DESCRIPTOR_DESC>& descriptors = pl_desc.descriptors;

			// SceneBVH
			RHI_DESCRIPTOR_DESC s_desc;
			s_desc.resource_type = resource_type_shader;
			s_desc.pool_range_start = 0;
			s_desc.pool_range_count = 1;
			descriptors.push_back(s_desc);

			// Output
			RHI_DESCRIPTOR_DESC o_desc;
			o_desc.resource_type = resource_type_generic_rw_buffer;
			o_desc.pool_range_start = 0;
			o_desc.pool_range_count = 1;
			descriptors.push_back(o_desc);

			// Camera
			RHI_DESCRIPTOR_DESC c_desc;
			c_desc.resource_type = resource_type_constant_buffer;
			c_desc.pool_range_start = 0;
			c_desc.pool_range_count = 1;
			descriptors.push_back(c_desc);

			pl_desc.shader_type = shader_type_undef;

			pipeline_layout = rhi_pipeline_layout_create(pl_desc);

			RHI_RT_PIPELINE_DESC p_desc(device, *pipeline_layout);

			auto& hg_1 = p_desc.hit_groups.emplace_back();
			hg_1.group_id = "HG_1";
			hg_1.closest_hit.blob = std::move(closest_hit_shader);
			hg_1.closest_hit.name_id = "ClosestHit";

			auto& miss_1 = p_desc.miss_shaders.emplace_back();
			miss_1.blob = std::move(miss_shader);
			miss_1.name_id = "Miss";

			p_desc.ray_gen.name_id = "RayGen";
			p_desc.ray_gen.blob = std::move(ray_gen_shader);

			pipeline = rhi_rt_pipeline_create(p_desc);

			// sbt
			RHI_RT_SBT_DESC sbt_desc;
			sbt_desc.ray_gen_ids.push_back("RayGen");
			sbt_desc.miss_ids.push_back("Miss");
			sbt_desc.hit_group_ids.push_back("HG_1");
			sbt = rhi_rt_pipeline_create_sbt(device, sbt_desc, *pipeline);

			// create geometry buffers
			RHI_VERTEX_BUFFER_DESC vb_desc(device);
			vb_desc.count = vertex_count;
			vb_desc.length = vertex_count * sizeof(Vertex);
			vb_desc.stride = sizeof(Vertex);
			vb_desc.format = resource_format_float3;
			vertex_buffer = rhi_buffers_create_vertices(vb_desc);

			RHI_INDEX_BUFFER_DESC ib_desc(device);
			ib_desc.count = index_count;
			ib_desc.length = sizeof(uint16_t) * index_count;
			ib_desc.stride = sizeof(uint16_t);
			ib_desc.format = resource_format_uint16;
			index_buffer = rhi_buffers_create_indices(ib_desc);

			// create shared memory for camera transforms
			RHI_BUFFER_DESC shared_camera_buffer_desc(device);
			shared_camera_buffer_desc.length = sizeof(CameraCBRT);
			shared_camera_buffer_desc.memory_type = buffer_memory_type_shared_rw;
			shared_camera_buffer_desc.default_state = resource_state_generic_read;
			shared_camera_buffer_desc.resource_slot = 2;
			shared_camera_constant_buffer = rhi_buffers_create_constant(shared_camera_buffer_desc);
			camera_constant_buffer_ptr = rhi_buffers_map_open(*shared_camera_constant_buffer, 0, sizeof(CameraCBRT));

			rhi_command_queue_execute(command_queue, true, [&](RHI_VOID_PTR UNUSED_PARAM(native_command_queue_impl),
				std::vector<RHI_COMMAND_BUFFER*>& command_buffer_list) {

					rhi_command_buffer_record(command_buffer, [&](RHI_VOID_PTR UNUSED_PARAM(native_command_buffer_impl)) {

						auto vertex_size = sizeof(Vertex);
						auto vertices_ptr = &vertices[0];

						// cpu bridge buffer uploading
						{
							RHI_BUFFER_DESC shared_buffer_desc(device);
							shared_buffer_desc.length = vb_desc.length;
							shared_buffer_desc.memory_type = buffer_memory_type_shared_rw;
							shared_buffer_desc.default_state = resource_state_generic_read;
							auto shared_vertex_buffer = rhi_buffers_create_raw(shared_buffer_desc);
							rhi_buffers_map_write(*shared_vertex_buffer, vertices_ptr, 0, shared_buffer_desc.length);
							rhi_buffers_gpu_upload(command_buffer, *shared_vertex_buffer, *vertex_buffer);
						}

						// cpu bridge buffer uploading
						{
							RHI_BUFFER_DESC shared_buffer_desc(device);
							shared_buffer_desc.length = ib_desc.length;
							shared_buffer_desc.memory_type = buffer_memory_type_shared_rw;
							shared_buffer_desc.default_state = resource_state_generic_read;
							auto shared_index_buffer = rhi_buffers_create_raw(shared_buffer_desc);
							rhi_buffers_map_write(*shared_index_buffer, &indices[0], 0, shared_buffer_desc.length);
							rhi_buffers_gpu_upload(command_buffer, *shared_index_buffer, *index_buffer);
						}

						RHI_RT_BVH_DESC blas_desc(device, command_buffer, *vertex_buffer, index_buffer.get());
						bvh = rhi_rt_bvh_create(blas_desc);

						RT_GEOMETRY_INSTANCES_DESC tlas_desc(device, command_buffer, *bvh);
						tlas_desc.transforms = { Eigen::Matrix4f::Identity() };
						tlas_desc.resource_slot = 0;
						bvh_instances = rhi_rt_bvh_build_geometry_instances(tlas_desc);

					});
					command_buffer_list.push_back(&command_buffer);
				});
		}	
		, [&](RHI_RENDER_PASS&) {
			// before draw
			rt_render_pass->set_pipeline(pipeline.get());
		}
		, [&] (RHI_DEVICE& device, RHI_RENDER_PASS& render_pass, RHI_COMMAND_BUFFER& command_buffer) {

			float dt = get_delta_time();
			auto world = rotate_triangle(dt);

			// draw
			memcpy(camera_constant_buffer_ptr, &camera, sizeof(CameraCBRT));

			rhi_render_pass_execute_rt_mode(*rt_render_pass, command_buffer, [&] {
				
				RT_GEOMETRY_INSTANCES_DESC tlas_desc(device, command_buffer, *bvh);
				tlas_desc.transforms = { world };
				tlas_desc.resource_slot = 0;
				bvh_instances = rhi_rt_bvh_build_geometry_instances(tlas_desc);

				rhi_command_buffer_ray_trace(device, command_buffer, *render_target, *pipeline, *bvh_instances, *sbt);
			});
			std::shared_ptr<RHI_TEXTURE_2D> back_buffer = rhi_swap_chain_get_surface(*swap_chain_ptr, -1);
			rhi_command_buffer_copy_texture(command_buffer, *back_buffer, *render_target);
		}
		, [&](RHI_RENDER_PASS& render_pass, RHI_SWAP_CHAIN& swap_chain, RHI_COMMAND_BUFFER& command_buffer) {

			// on_before_present
			
		},
		[&](RHI_DEVICE& UNUSED_PARAM(device))
			{
				// if (on_end)
				//	on_end(device);

				// on end
				rhi_buffers_map_close(*shared_camera_constant_buffer, 0, sizeof(CameraCBRT));
			}
		);
}