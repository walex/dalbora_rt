#include "test_api.hpp"
#include "rhi.hpp"

#include "dx12_rhi.hpp"

std::unique_ptr<D3D12_CPU_DESCRIPTOR_HANDLE> dx12_helpers_get_rw_descriptor_heap_handle(ID3D12Device* device, ID3D12DescriptorHeap* heap, size_t slot) {

	D3D12_DESCRIPTOR_HEAP_DESC desc = heap->GetDesc();
	if (slot + 1 > (int)desc.NumDescriptors) {
		throw std::exception("Max descriptors reached for type %d", desc.Type);
	}
	UINT rtvDescriptorSize =
		device->GetDescriptorHandleIncrementSize(desc.Type);
	auto h = heap->GetCPUDescriptorHandleForHeapStart();
	h.ptr += (slot * rtvDescriptorSize);
	return std::make_unique<D3D12_CPU_DESCRIPTOR_HANDLE>(h);
}

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

	std::unique_ptr<D3D12_CPU_DESCRIPTOR_HANDLE> srv_handle;
	std::unique_ptr<D3D12_CPU_DESCRIPTOR_HANDLE> cvb_handle;
	
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

	test_swap_chain([&](RHI_DEVICE& dev, RHI_COMMAND_QUEUE& command_queue,
		RHI_COMMAND_BUFFER& command_buffer, RHI_SWAP_CHAIN& swap_chain)
		{
			// BORRAR
			DX_DEVICE& dxdev = static_cast<DX_DEVICE&>(dev);
			ID3D12Device* i_device = *static_cast<DX_DEVICE*>(&dev);
			ID3D12DescriptorHeap* i_heap = *dxdev.resources_heap.get();
			// FIN BORRAR

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
			
			// BORRAR

			
			DX_TEXTURE_2D* rt = static_cast<DX_TEXTURE_2D*>(render_target.get());			
			ID3D12Resource* i_texture = static_cast<ID3D12Resource*>(rt->com_ptr.Get());
			std::unique_ptr<D3D12_CPU_DESCRIPTOR_HANDLE> srv_handle = dx12_helpers_get_rw_descriptor_heap_handle(i_device, i_heap, 1);
			
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
			RHI_DESCRIPTOR_DESC& s_desc = pl_desc.descriptors[pl_desc.descriptor_count++];
			s_desc.resource_type = resource_type_shader;
			s_desc.shader_register_start = 0;
			s_desc.shader_register_max = 100; // max registers for this type, can be used for any resource of this type,
												// just need to specify the correct register in the shader

			// 2 - GPU read write (Render Target)
			RHI_DESCRIPTOR_DESC& o_desc = pl_desc.descriptors[pl_desc.descriptor_count++];
			o_desc.resource_type = resource_type_generic_rw_buffer;
			o_desc.shader_register_start = 0;
			o_desc.shader_register_max = 100; // max registers for this type, can be used for any resource of this type,
												// just need to specify the correct register in the shader

			// 3 - Constant buffer (Camera)
			RHI_DESCRIPTOR_DESC& c_desc = pl_desc.descriptors[pl_desc.descriptor_count++];
			c_desc.resource_type = resource_type_constant_buffer;
			c_desc.shader_register_start = 0;
			c_desc.shader_register_max = 100; // max registers for this type, can be used for any resource of this type,
												// just need to specify the correct register in the shader

			pipeline_layout.reset(rhi_pipeline_layout_create(&pl_desc));

			RHI_RT_PIPELINE_DESC p_desc;
			p_desc.device = &dev;
			p_desc.layout = pipeline_layout.get();

			const char ray_gen_id[16] = "RayGen";
			const char miss_id[16] = "Miss";
			const char hit_group_id_0[16] = "HG_1";
			const char group_id_0_closest_hit_id[16] = "ClosestHit";

			auto& hg_1 = p_desc.hit_groups[p_desc.hit_group_count++];
			strcpy_s(hg_1.group_id, hit_group_id_0);
			hg_1.closest_hit.blob = closest_hit_shader;
			strcpy_s(hg_1.closest_hit.name_id, group_id_0_closest_hit_id);

			auto& miss_1 = p_desc.miss_shaders[p_desc.miss_shader_count++];
			miss_1.blob = miss_shader;
			strcpy_s(miss_1.name_id, miss_id);

			auto& ran_gen = p_desc.ray_gen[p_desc.ray_gen_count++];
			strcpy(ran_gen.name_id, ray_gen_id);
			ran_gen.blob = ray_gen_shader;

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
						blas_desc.vertex_buffer = vertex_buffer.get();
						blas_desc.index_buffer = index_buffer.get();
						bvh.reset(rhi_rt_bvh_create(&blas_desc));

						RT_GEOMETRY_INSTANCES_DESC tlas_desc;
						tlas_desc.device = &dev;
						tlas_desc.command_buffer = &command_buffer;
						tlas_desc.parent_bvh = bvh.get();
						tlas_desc.transforms = &rotation_matrix;
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
			rt_instances_view_desc.buffer = static_cast<DX_TEXTURE_2D*>(render_target.get());
			rt_instances_view_desc.type = resource_type_texture_2d_rw;
			rt_instances_view_desc.slot_id = 100;
			render_target_view.reset(rhi_buffers_create_view(&rt_instances_view_desc));

			// view camera (constant buffer)
			RHI_VIEW_DESC cb_instances_view_desc;
			cb_instances_view_desc.format = tx_desc.format;
			cb_instances_view_desc.device = &dev;
			cb_instances_view_desc.buffer = static_cast<DX_TEXTURE_2D*>(shared_camera_constant_buffer.get());
			cb_instances_view_desc.type = resource_type_constant_buffer;
			cb_instances_view_desc.slot_id = 200;
			camera_constant_buffer_view.reset(rhi_buffers_create_view(&cb_instances_view_desc));

			// create render pass
			RHI_RENDER_PASS_DESC render_pass_desc;
			render_pass_desc.device = &dev;
			render_pass_desc.render_target_view = render_target_view.get();
			rt_render_pass.reset(rhi_render_pass_create(&render_pass_desc));
		}	
		, [&](RHI_RENDER_PASS&) {
			// before draw

			rt_render_pass->pipeline = pipeline.get();
			
		}
		, [&] (RHI_DEVICE& dev, RHI_RENDER_PASS& render_pass, RHI_COMMAND_BUFFER& command_buffer) {

			float dt = get_delta_time();
			auto world = rotate_triangle(dt);

			//RT_GEOMETRY_INSTANCES_DESC tlas_desc;
			//tlas_desc.device = &dev;
			//tlas_desc.command_buffer = &command_buffer;
			//tlas_desc.parent_bvh = bvh.get();
			//tlas_desc.transforms = &rotation_matrix;
			//tlas_desc.instance_count = 1;
			//bvh_instances.reset(rhi_rt_bvh_build_geometry_instances(&tlas_desc));

			//// view
			//rhi_buffers_update_view(&dev, bvh_instances_view.get(), bvh_instances.get());

			// draw
			memcpy(camera_constant_buffer_ptr, &camera, sizeof(CameraCBRT));

			rhi_render_pass_execute_rt_mode(rt_render_pass.get(), &command_buffer, [&] {				
		
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
				// if (on_end)
				//	on_end(device);

				rhi_buffers_map_close(shared_camera_constant_buffer.get(), 0, sizeof(CameraCBRT));
			}
		);
}