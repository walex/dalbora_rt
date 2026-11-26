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

void test_rt_triangle() {

	constexpr size_t MAX_COMMAND_ALLOCATORS = 8;

	std::unique_ptr<RHI_DEVICE> device;
	{
		std::unique_ptr<RHI_COMMAND_QUEUE> command_queue;
		std::unique_ptr<RHI_SWAP_CHAIN> swap_chain;
		std::unique_ptr<RHI_COMMAND_BUFFER> command_buffer;
		std::unique_ptr<RHI_RENDER_PASS> render_pass;
		std::vector<std::unique_ptr<RHI_VIEW>> views;
		std::vector<std::unique_ptr<RHI_MEMORY_DESCRIPTOR_SLOT>> views_slots;
		RHI_COMMAND_ALLOCATOR_POOL* command_allocator_pool[queue_type_count];

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
		std::unique_ptr<RHI_MEMORY_DESCRIPTOR_SLOT> bvh_descriptor_slot;
		std::unique_ptr<RHI_MEMORY_DESCRIPTOR_SLOT> render_target_descriptor_slot;
		std::unique_ptr<RHI_MEMORY_DESCRIPTOR_SLOT> camera_constant_buffer_descriptor_slot;
		RHI_VOID_PTR camera_constant_buffer_ptr;
		float4x4 rotation_matrix = float4x4::Identity();

		
		ObjectCB triangle_transforms;
		triangle_transforms.world = float4x4::Identity();

		CameraCBRT camera;
		camera.camera_pos =
			float4(0.0f, 0.0f, -3.0f, 1.0f);

		camera.camera_forward =
			float4(0.0f, 0.0f, 1.0f, 0.0f);

		camera.camera_right =
			float4(1.0f, 0.0f, 0.0f, 0.0f);

		camera.camera_up =
			float4(0.0f, 1.0f, 0.0f, 0.0f);

		camera.tanHalfFov =
			0.7002075f;

		camera.aspect = image_aspect;

		const std::filesystem::path shaders_folder = get_executable_folder("shaders");

		std::shared_ptr<RHI_WINDOW_CALLBACKS> callbacks = std::make_shared<RHI_WINDOW_CALLBACKS>();

		callbacks.get()->on_init = ([&](RHI_WINDOW* const window) {
			RHI_DEVICE_DESC device_desc;
			device_desc.adapter_id = -1;
			device_desc.features = device_features_raytracing;
			device_desc.app_instance = rhi_get_app_instance();
			device_desc.graphics_queue_count = 1;
			device_desc.compute_queue_count = 1;
			device_desc.copy_queue_count = 1;
			device_desc.shader_model = hlsl_shader_model_6_8;
			device.reset(rhi_create_device(&device_desc));

			memory_descriptors_init(device.get());

			command_allocator_pool[queue_type_graphics] = rhi_command_allocator_pool_create(device.get(), MAX_COMMAND_ALLOCATORS, queue_type_graphics);
			command_allocator_pool[queue_type_compute] = rhi_command_allocator_pool_create(device.get(), MAX_COMMAND_ALLOCATORS, queue_type_compute);
			command_allocator_pool[queue_type_copy] = rhi_command_allocator_pool_create(device.get(), MAX_COMMAND_ALLOCATORS, queue_type_copy);

			RHI_COMMAND_QUEUE_DESC queue_desc;
			queue_desc.device = device.get();
			queue_desc.type = queue_type_graphics;
			command_queue.reset(rhi_command_queue_create(&queue_desc));

			RHI_SWAP_CHAIN_DESC swap_chain_desc;
			swap_chain_desc.device = device.get();
			swap_chain_desc.command_queue = command_queue.get();
			swap_chain_desc.window = window;
			swap_chain_desc.width = 800;
			swap_chain_desc.height = 600;
			swap_chain_desc.enable_vsync = false;
			swap_chain_desc.is_full_screen = false;
			swap_chain_desc.buffer_count = 3;
			swap_chain_desc.color_format = resource_format_R8G8B8A8_norm;
			swap_chain.reset(rhi_swap_chain_create(&swap_chain_desc));

			for (size_t i = 0; i < swap_chain_desc.buffer_count; i++) {

				views_slots.emplace_back(*get_rtv_memory_descriptors().next_descriptor_ptr());
				RHI_VIEW* view_ptr = rhi_swap_chain_create_view(device.get(), swap_chain.get(), views_slots.back().get(), swap_chain_desc.color_format, i);
				views.push_back(std::unique_ptr<RHI_VIEW>(view_ptr));
			}

			RHI_COMMAND_BUFFER_DESC command_buffer_desc;
			command_buffer_desc.device = device.get();
			command_buffer_desc.command_queue = command_queue.get();
			command_buffer_desc.command_allocator = rhi_command_allocator_pool_acquire(command_allocator_pool[queue_type_graphics]);
			command_buffer.reset(rhi_command_buffer_create(&command_buffer_desc));
			command_buffer->buffer_memory_descriptor = get_buffers_memory_descriptors();

			RHI_VIEWPORT vp;
			vp.x = 0;
			vp.y = 0;
			vp.width = swap_chain->buffer_width;
			vp.height = swap_chain->buffer_height;
			vp.min_z = 0.0f;
			vp.max_z = 1.0f;

			RHI_RENDER_PASS_DESC render_pass_desc;
			render_pass_desc.device = device.get();
			render_pass.reset(rhi_render_pass_create(&render_pass_desc));
			render_pass->view_port = vp;


			RhiMemoryDescriptors& resources_memory_descriptor = get_buffers_memory_descriptors();

			RHI_TEXTURE_2D_DESC tx_desc;
			tx_desc.device = device.get();
			tx_desc.memory_type = buffer_memory_type_gpu_read_only;
			tx_desc.type = buffer_type_image_2d;
			tx_desc.width = swap_chain->buffer_width;
			tx_desc.height = swap_chain->buffer_height;
			tx_desc.format = resource_format_R8G8B8A8_norm;
			tx_desc.mips = 1;
			tx_desc.resource_flags = resource_flags_shader_read_write | resource_flags_render_target;
			render_target.reset(rhi_texture_2d_create(&tx_desc));

			// compile shaders
			std::string rt_file = std::filesystem::path(shaders_folder / "simple_rt.hlsl").string();
			int flags = byte_code_format_input_hlsl;
			if (rhi_get_device_type() == rhi_device_type_vulkan) {
				flags |= byte_code_format_output_spirv;
			}

			auto ray_gen_shader = rhi_hlsl_compiler_compile(rt_file.c_str(), "RayGen", "lib_6_6", shaders_folder.string().c_str(), static_cast<byte_code_format>(flags));
			auto miss_shader = rhi_hlsl_compiler_compile(rt_file.c_str(), "Miss", "lib_6_6", shaders_folder.string().c_str(), static_cast<byte_code_format>(flags));
			auto closest_hit_shader = rhi_hlsl_compiler_compile(rt_file.c_str(), "ClosestHit", "lib_6_6", shaders_folder.string().c_str(), static_cast<byte_code_format>(flags));

			RHI_PIPELINE_LAYOUT_DESC pl_desc;
			pl_desc.device = device.get();

			// descriptors ( order mathers )

			// 1 - GPU read only (Scene BVH)
			RHI_SHADER_DESCRIPTOR_DESC& s_desc = pl_desc.descriptors[pl_desc.descriptor_count++];
			s_desc.shader_view_type = shader_view_type_read_only_buffer;
			s_desc.shader_register_start = 0;
			s_desc.shader_register_max = 5; // max registers for this type, can be used for any resource of this type,
			// just need to specify the correct register in the shader

			// 2 - GPU read write (Render Target)
			RHI_SHADER_DESCRIPTOR_DESC& o_desc = pl_desc.descriptors[pl_desc.descriptor_count++];
			o_desc.shader_view_type = shader_view_type_rw_buffer;
			o_desc.shader_register_start = 0;
			o_desc.shader_register_max = 5; // max registers for this type, can be used for any resource of this type,
			// just need to specify the correct register in the shader

			// 3 - Constant buffer (Camera)
			RHI_SHADER_DESCRIPTOR_DESC& c_desc = pl_desc.descriptors[pl_desc.descriptor_count++];
			c_desc.shader_view_type = shader_view_type_constant_buffer;
			c_desc.shader_register_start = 0;
			c_desc.shader_register_max = 100; // max registers for this type, can be used for any resource of this type,
			// just need to specify the correct register in the shader

			pipeline_layout.reset(rhi_pipeline_layout_create(&pl_desc));

			// rt pipeline config
			RHI_RT_PIPELINE_DESC p_desc;
			p_desc.device = device.get();
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
			strcpy_s(ray_gen.name_id, ray_gen_id);
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
			sbt.reset(rhi_rt_pipeline_create_sbt(device.get(), &sbt_desc, pipeline.get()));

			// create geometry buffers
			RHI_VERTEX_BUFFER_DESC vb_desc;
			vb_desc.device = device.get();
			vb_desc.count = vertex_count;
			vb_desc.length = vertex_count * sizeof(Vertex);
			vb_desc.stride = sizeof(Vertex);
			vb_desc.format = resource_format_float3;
			vb_desc.memory_type = buffer_memory_type_gpu_read_only;
			vb_desc.type = buffer_type_raw;
			vb_desc.resource_flags |= resource_flags_rt_as;
			vertex_buffer.reset(rhi_buffers_create_vertices(&vb_desc));

			RHI_INDEX_BUFFER_DESC ib_desc;
			ib_desc.device = device.get();
			ib_desc.count = index_count;
			ib_desc.length = sizeof(uint16_t) * index_count;
			ib_desc.stride = sizeof(uint16_t);
			ib_desc.format = resource_format_uint16;
			ib_desc.memory_type = buffer_memory_type_gpu_read_only;
			ib_desc.type = buffer_type_raw;
			ib_desc.resource_flags |= resource_flags_rt_as;
			index_buffer.reset(rhi_buffers_create_indices(&ib_desc));

			// create shared memory for camera transforms
			RHI_BUFFER_DESC shared_camera_buffer_desc;
			shared_camera_buffer_desc.device = device.get();
			shared_camera_buffer_desc.length = sizeof(CameraCBRT);
			shared_camera_buffer_desc.memory_type = buffer_memory_type_shared_rw;
			shared_camera_buffer_desc.type = buffer_type_raw;
			shared_camera_buffer_desc.mips = 1;
			shared_camera_constant_buffer.reset(rhi_buffers_create_constant(&shared_camera_buffer_desc));

			// open cb for write
			camera_constant_buffer_ptr = rhi_buffers_map_open(shared_camera_constant_buffer.get(), 0, sizeof(CameraCBRT));

			std::unique_ptr<RHI_BUFFER> shared_vertex_buffer;
			std::unique_ptr<RHI_BUFFER> shared_index_buffer;

			rhi_command_queue_execute(command_queue.get(), true, [&](RHI_VOID_PTR UNUSED_PARAM(native_command_queue_impl),
				std::vector<RHI_COMMAND_BUFFER*>* command_buffer_list) {

					rhi_command_buffer_record(command_buffer.get(), [&](RHI_VOID_PTR UNUSED_PARAM(native_command_buffer_impl)) {

						auto vertices_ptr = &vertices[0];

						// cpu bridge buffer uploading
						{
							RHI_BUFFER_DESC shared_buffer_desc;
							shared_buffer_desc.device = device.get();
							shared_buffer_desc.length = vb_desc.length;
							shared_buffer_desc.memory_type = buffer_memory_type_shared_rw;
							shared_buffer_desc.type = buffer_type_raw;
							shared_buffer_desc.mips = 1;
							shared_vertex_buffer.reset(rhi_buffers_create_raw(&shared_buffer_desc));
							rhi_buffers_map_write(shared_vertex_buffer.get(), vertices_ptr, 0, shared_buffer_desc.length);
							rhi_buffers_gpu_upload(command_buffer.get(), shared_vertex_buffer.get(), vertex_buffer.get());
						}

						// cpu bridge buffer uploading
						{
							RHI_BUFFER_DESC shared_buffer_desc;
							shared_buffer_desc.device = device.get();
							shared_buffer_desc.length = ib_desc.length;
							shared_buffer_desc.memory_type = buffer_memory_type_shared_rw;
							shared_buffer_desc.type = buffer_type_raw;
							shared_buffer_desc.mips = 1;
							shared_index_buffer.reset(rhi_buffers_create_raw(&shared_buffer_desc));
							rhi_buffers_map_write(shared_index_buffer.get(), &indices[0], 0, shared_buffer_desc.length);
							rhi_buffers_gpu_upload(command_buffer.get(), shared_index_buffer.get(), index_buffer.get());
						}

						RHI_RT_BVH_DESC blas_desc;
						blas_desc.device = device.get();
						blas_desc.command_buffer = command_buffer.get();
						RHI_BUFFER* vb = vertex_buffer.get();
						RHI_BUFFER* ib = index_buffer.get();
						blas_desc.vertex_buffer = &vb;
						blas_desc.index_buffer = &ib;
						blas_desc.count = 1;
						bvh.reset(rhi_rt_bvh_create(&blas_desc));

						float* matrices[] = { reinterpret_cast<float*>(&rotation_matrix) };


						RHI_RT_BVH_GEOMETRY_INSTANCE_DESC inst_desc;

						RHI_RT_BVH_GEOMETRY_DESC tlas_desc;
						tlas_desc.device = device.get();
						tlas_desc.command_buffer = command_buffer.get();
						tlas_desc.instance_info = &inst_desc;
						inst_desc.parent_bvh = bvh.get();
						inst_desc.transforms = &matrices[0];
						inst_desc.transforms_count = 1;
						tlas_desc.read_only = false;
						tlas_desc.instance_info_count = 1;
						tlas_desc.total_instances = 1;
						bvh_instances.reset(rhi_rt_bvh_build_geometry_instances(&tlas_desc));
						});
					command_buffer_list->push_back(command_buffer.get());
				});


			// view BVH (GPU read only)
			RHI_VIEW_DESC bvh_instances_view_desc;
			bvh_instances_view_desc.device = device.get();
			bvh_instances_view_desc.buffer = bvh_instances.get();
			bvh_instances_view_desc.type = shader_view_type_bvh_buffer;

			RHI_MEMORY_DESCRIPTOR_SLOT* slot = nullptr;
			resources_memory_descriptor.next_descriptor_ptr(&slot);
			ASSERT_PTR(slot);
			bvh_descriptor_slot.reset(slot);

			bvh_instances_view_desc.memory_descriptor = bvh_descriptor_slot.get();
			bvh_instances_view.reset(rhi_buffers_create_view(&bvh_instances_view_desc));

			// view render_target (GPU read write)
			RHI_VIEW_DESC rt_instances_view_desc;
			rt_instances_view_desc.format = tx_desc.format;
			rt_instances_view_desc.device = device.get();
			rt_instances_view_desc.buffer = dynamic_cast<RHI_BUFFER*>(render_target.get());
			rt_instances_view_desc.type = shader_view_type_rw_texture_buffer;

			slot = nullptr;
			resources_memory_descriptor.next_descriptor_ptr(&slot, 1);
			ASSERT_PTR(slot);
			render_target_descriptor_slot.reset(slot);

			rt_instances_view_desc.memory_descriptor = render_target_descriptor_slot.get();
			render_target_view.reset(rhi_buffers_create_view(&rt_instances_view_desc));

			// view camera (constant buffer)
			RHI_VIEW_DESC cb_instances_view_desc;
			cb_instances_view_desc.format = tx_desc.format;
			cb_instances_view_desc.device = device.get();
			cb_instances_view_desc.buffer = static_cast<RHI_BUFFER*>(shared_camera_constant_buffer.get());
			cb_instances_view_desc.type = shader_view_type_constant_buffer;

			slot = nullptr;
			resources_memory_descriptor.next_descriptor_ptr(&slot, 2);
			ASSERT_PTR(slot);
			camera_constant_buffer_descriptor_slot.reset(slot);

			cb_instances_view_desc.memory_descriptor = camera_constant_buffer_descriptor_slot.get();
			camera_constant_buffer_view.reset(rhi_buffers_create_view(&cb_instances_view_desc));

			// create render pass
			RHI_RENDER_PASS_DESC rt_render_pass_desc;
			rt_render_pass_desc.device = device.get();
			rt_render_pass.reset(rhi_render_pass_create(&rt_render_pass_desc));
			rt_render_pass->render_target_view = render_target_view.get();

		});

		callbacks.get()->on_idle = ([&](const RHI_WINDOW* UNUSED_PARAM(window)) {

			rt_render_pass->pipeline = pipeline.get();

			float dt = get_delta_time();
			rotation_matrix = rotate_triangle(dt);
			float* matrices[] = { reinterpret_cast<float*>(&rotation_matrix) };
			RHI_RT_BVH_GEOMETRY_INSTANCE_DESC inst_desc;
			RHI_RT_BVH_GEOMETRY_DESC tlas_desc;
			tlas_desc.device = device.get();
			tlas_desc.command_buffer = command_buffer.get();
			tlas_desc.instance_info = &inst_desc;
			inst_desc.parent_bvh = bvh.get();
			inst_desc.transforms = &matrices[0];
			inst_desc.transforms_count = 1;
			tlas_desc.instance_info_count = 1;
			tlas_desc.read_only = false;
			tlas_desc.total_instances = 1;
		

			swap_chain->current_image_index = rhi_swap_chain_get_current_buffer_id(swap_chain.get());
			command_buffer->rtv_memory_descriptor = get_rtv_memory_descriptors();
			command_buffer->dsv_memory_descriptor = get_dsv_memory_descriptors();
			render_pass->render_target_view = views[swap_chain->current_image_index].get();

			rhi_command_queue_execute(command_queue.get(), true, [&](
				RHI_VOID_PTR UNUSED_PARAM(native_command_queue_impl),
				std::vector<RHI_COMMAND_BUFFER*>* const command_buffer_list) {

					rhi_command_buffer_record(command_buffer.get(),
						[&](RHI_VOID_PTR UNUSED_PARAM(native_command_buffer_impl)) {

						rhi_rt_bvh_update_geometry_instances(&tlas_desc, bvh_instances.get());
							
						rhi_render_pass_execute_rt_mode(rt_render_pass.get(), command_buffer.get(), [&] {
								
							rhi_command_buffer_ray_trace(
								command_buffer.get(),
								render_target.get(),
								sbt.get());
						});

						memcpy(camera_constant_buffer_ptr, &camera, sizeof(CameraCBRT));
						rhi_command_buffer_copy_texture(command_buffer.get(), dynamic_cast<RHI_TEXTURE_2D*>(render_pass->render_target_view->buffer.get()), render_target.get());
					});
					command_buffer_list->push_back(command_buffer.get());
				});

				// present
				rhi_swap_chain_present(swap_chain.get());
			});

		callbacks.get()->on_end = ([&](const RHI_WINDOW* UNUSED_PARAM(window)) {
			rhi_buffers_map_close(shared_camera_constant_buffer.get(), 0, sizeof(CameraCBRT));
		});

		// setup window
		RHI_WINDOW_DESC window_desc;
		window_desc.full_screen = false;
		window_desc.width = 800;
		window_desc.height = 600;
		const char* title = "pbr test for create window";
		window_desc.title = const_cast<char*>(title);
		window_desc.callbacks = callbacks.get();

		// create window
		std::unique_ptr<RHI_WINDOW> window(rhi_create_window(&window_desc));

		// main loop
		rhi_window_main_loop(window.get());

		// release objects in order
		RHI_COMMAND_ALLOCATOR* command_allocator = command_buffer->command_allocator;

		command_buffer.reset();
		command_queue.reset();
		render_pass.reset();

		rhi_command_allocator_pool_release(command_allocator_pool[queue_type_graphics], command_allocator);
		rhi_command_allocator_pool_clean(command_allocator_pool[queue_type_graphics]);
		rhi_command_allocator_pool_clean(command_allocator_pool[queue_type_compute]);
		rhi_command_allocator_pool_clean(command_allocator_pool[queue_type_copy]);
		delete command_allocator_pool[queue_type_graphics];
		delete command_allocator_pool[queue_type_compute];
		delete command_allocator_pool[queue_type_copy];
		swap_chain.reset();

		buffers_memory_table.reset();
		rtv_memory_table.reset();
		dsv_memory_table.reset();
		samplers_memory_table.reset();
	}
}

//
//void test_rt_triangle(fptr_test_on_init UNUSED_PARAM(on_init),
//	fptr_test_on_draw UNUSED_PARAM(on_draw),
//	fptr_test_on_end UNUSED_PARAM(on_end),
//	fptr_test_on_layout UNUSED_PARAM(on_layout),
//	fptr_test_on_configure_device UNUSED_PARAM(on_configure_device))
//{
//
//	std::unique_ptr<RHI_BUFFER> vertex_buffer;
//	std::unique_ptr<RHI_BUFFER> index_buffer;
//	std::unique_ptr<RHI_PIPELINE_LAYOUT> pipeline_layout;
//	std::shared_ptr<RHI_TEXTURE_2D> render_target;
//	std::shared_ptr<RHI_VIEW> render_target_view;
//	std::unique_ptr<RHI_RENDER_PASS> rt_render_pass;
//	std::unique_ptr<RHI_RT_BVH> bvh;
//	std::unique_ptr<RHI_BUFFER> bvh_instances;
//	std::unique_ptr<RHI_VIEW> bvh_instances_view;
//	std::unique_ptr<RHI_SBT_TABLE> sbt;
//	std::unique_ptr<RHI_RT_PIPELINE> pipeline;
//	std::unique_ptr<RHI_BUFFER> shared_camera_constant_buffer;
//	std::unique_ptr<RHI_VIEW> camera_constant_buffer_view;
//	std::unique_ptr<RHI_MEMORY_DESCRIPTOR_SLOT> bvh_descriptor_slot;
//	std::unique_ptr<RHI_MEMORY_DESCRIPTOR_SLOT> render_target_descriptor_slot;
//	std::unique_ptr<RHI_MEMORY_DESCRIPTOR_SLOT> camera_constant_buffer_descriptor_slot;
//	RHI_COMMAND_BUFFER* command_buffer_ptr = nullptr;
//	RHI_DEVICE* device_ptr = nullptr;
//
//	float4x4 rotation_matrix = float4x4::Identity();
//
//	RHI_VOID_PTR camera_constant_buffer_ptr;
//
//	const std::filesystem::path shaders_folder = get_executable_folder("shaders");
//
//	RHI_SWAP_CHAIN* swap_chain_ptr;
//
//	CameraCBRT camera;
//	camera.camera_pos =
//		float4(0.0f, 0.0f, -3.0f, 1.0f);
//
//	camera.camera_forward =
//		float4(0.0f, 0.0f, 1.0f, 0.0f);
//
//	camera.camera_right =
//		float4(1.0f, 0.0f, 0.0f, 0.0f);
//
//	camera.camera_up =
//		float4(0.0f, 1.0f, 0.0f, 0.0f);
//
//	camera.tanHalfFov =
//		0.7002075f;
//
//	camera.aspect = image_aspect;
//
//	test_swap_chain([&](RHI_DEVICE& dev, RHI_COMMAND_QUEUE& command_queue,
//		RHI_COMMAND_BUFFER& command_buffer, RHI_SWAP_CHAIN& swap_chain, 
//		RHI_COMMAND_ALLOCATOR_POOL* command_allocator_pool[queue_type_count])
//		{
//			command_buffer_ptr = &command_buffer;
//			device_ptr = &dev;
//
//			RhiMemoryTable& resources_memory_descriptor = get_buffers_memory_table();
//
//			swap_chain_ptr = &swap_chain;
//
//			RHI_TEXTURE_2D_DESC tx_desc;
//			tx_desc.device = &dev;
//			tx_desc.memory_type = buffer_memory_type_gpu_read_only;
//			tx_desc.type = buffer_type_image_2d;
//			tx_desc.width = swap_chain.buffer_width;
//			tx_desc.height = swap_chain.buffer_height;
//			tx_desc.format = resource_format_R8G8B8A8_norm;
//			tx_desc.mips = 1;
//			tx_desc.resource_flags = resource_flags_shader_read_write | resource_flags_render_target;
//			render_target.reset(rhi_texture_2d_create(&tx_desc));
//			
//			// compile shaders
//			std::string rt_file = std::filesystem::path(shaders_folder / "simple_rt.hlsl").string();
//			int flags = byte_code_format_input_hlsl;
//			if (rhi_get_device_type() == rhi_device_type_vulkan) {
//				flags |= byte_code_format_output_spirv;
//			}
//
//			auto ray_gen_shader = rhi_hlsl_compiler_compile(rt_file.c_str(), "RayGen", "lib_6_6", shaders_folder.string().c_str(), static_cast<byte_code_format>(flags));
//			auto miss_shader = rhi_hlsl_compiler_compile(rt_file.c_str(), "Miss", "lib_6_6", shaders_folder.string().c_str(), static_cast<byte_code_format>(flags));
//			auto closest_hit_shader = rhi_hlsl_compiler_compile(rt_file.c_str(),  "ClosestHit", "lib_6_6", shaders_folder.string().c_str(), static_cast<byte_code_format>(flags));
//
//			RHI_PIPELINE_LAYOUT_DESC pl_desc;
//			pl_desc.device = &dev;
//
//			// descriptors ( order mathers )
//
//			// 1 - GPU read only (Scene BVH)
//			RHI_SHADER_DESCRIPTOR_DESC& s_desc = pl_desc.descriptors[pl_desc.descriptor_count++];
//			s_desc.shader_view_type = shader_view_type_read_only_buffer;
//			s_desc.shader_register_start = 0;
//			s_desc.shader_register_max = 5; // max registers for this type, can be used for any resource of this type,
//												// just need to specify the correct register in the shader
//
//			// 2 - GPU read write (Render Target)
//			RHI_SHADER_DESCRIPTOR_DESC& o_desc = pl_desc.descriptors[pl_desc.descriptor_count++];
//			o_desc.shader_view_type = shader_view_type_rw_buffer;
//			o_desc.shader_register_start = 0;
//			o_desc.shader_register_max = 5; // max registers for this type, can be used for any resource of this type,
//												// just need to specify the correct register in the shader
//
//			// 3 - Constant buffer (Camera)
//			RHI_SHADER_DESCRIPTOR_DESC& c_desc = pl_desc.descriptors[pl_desc.descriptor_count++];
//			c_desc.shader_view_type = shader_view_type_constant_buffer;
//			c_desc.shader_register_start = 0;
//			c_desc.shader_register_max = 100; // max registers for this type, can be used for any resource of this type,
//												// just need to specify the correct register in the shader
//
//			pipeline_layout.reset(rhi_pipeline_layout_create(&pl_desc));
//
//			// rt pipeline config
//			RHI_RT_PIPELINE_DESC p_desc;
//			p_desc.device = &dev;
//			p_desc.layout = pipeline_layout.get();
//
//			const char ray_gen_id[16] = "RayGen";
//			const char miss_id[16] = "Miss";
//			const char hit_group_id_0[16] = "HG_1";
//			const char group_id_0_closest_hit_id[16] = "ClosestHit";
//
//			auto& hg_1 = p_desc.hit_groups[p_desc.hit_group_count++];
//			strcpy_s(hg_1.name_id, hit_group_id_0);
//			hg_1.closest_hit.blob = closest_hit_shader;
//			strcpy_s(hg_1.closest_hit.name_id, group_id_0_closest_hit_id);
//
//			auto& miss_1 = p_desc.miss_shaders[p_desc.miss_shader_count++];
//			miss_1.blob = miss_shader;
//			strcpy_s(miss_1.name_id, miss_id);
//
//			auto& ray_gen = p_desc.ray_gen[p_desc.ray_gen_count++];
//			strcpy_s(ray_gen.name_id, ray_gen_id);
//			ray_gen.blob = ray_gen_shader;
//
//			pipeline.reset(rhi_rt_pipeline_create(&p_desc));
//
//			// sbt			
//			RHI_RT_SBT_DESC sbt_desc;
//			sbt_desc.ray_gen_ids[0] = ray_gen_id;
//			sbt_desc.ray_gen_count = 1;
//			sbt_desc.miss_ids[0] = miss_id;
//			sbt_desc.miss_shader_count = 1;
//			sbt_desc.hit_group_ids[0] = hit_group_id_0;
//			sbt_desc.hit_group_count = 1;
//			sbt.reset(rhi_rt_pipeline_create_sbt(&dev, &sbt_desc, pipeline.get()));
//
//			// create geometry buffers
//			RHI_VERTEX_BUFFER_DESC vb_desc;
//			vb_desc.device = &dev;
//			vb_desc.count = vertex_count;
//			vb_desc.length = vertex_count * sizeof(Vertex);
//			vb_desc.stride = sizeof(Vertex);
//			vb_desc.format = resource_format_float3;
//			vb_desc.memory_type = buffer_memory_type_gpu_read_only;
//			vb_desc.type = buffer_type_raw;
//			vb_desc.resource_flags |= resource_flags_rt_as;
//			vertex_buffer.reset(rhi_buffers_create_vertices(&vb_desc));
//
//			RHI_INDEX_BUFFER_DESC ib_desc;
//			ib_desc.device = &dev;
//			ib_desc.count = index_count;
//			ib_desc.length = sizeof(uint16_t) * index_count;
//			ib_desc.stride = sizeof(uint16_t);
//			ib_desc.format = resource_format_uint16;
//			ib_desc.memory_type = buffer_memory_type_gpu_read_only;
//			ib_desc.type = buffer_type_raw;
//			ib_desc.resource_flags |= resource_flags_rt_as;
//			index_buffer.reset(rhi_buffers_create_indices(&ib_desc));
//
//			// create shared memory for camera transforms
//			RHI_BUFFER_DESC shared_camera_buffer_desc;
//			shared_camera_buffer_desc.device = &dev;
//			shared_camera_buffer_desc.length = sizeof(CameraCBRT);
//			shared_camera_buffer_desc.memory_type = buffer_memory_type_shared_rw;
//			shared_camera_buffer_desc.type = buffer_type_raw;
//			shared_camera_buffer_desc.mips = 1;
//			shared_camera_constant_buffer.reset(rhi_buffers_create_constant(&shared_camera_buffer_desc));
//			
//			// open cb for write
//			camera_constant_buffer_ptr = rhi_buffers_map_open(shared_camera_constant_buffer.get(), 0, sizeof(CameraCBRT));
//
//			std::unique_ptr<RHI_BUFFER> shared_vertex_buffer;
//			std::unique_ptr<RHI_BUFFER> shared_index_buffer;
//
//			rhi_command_queue_execute(&command_queue, true, [&](RHI_VOID_PTR UNUSED_PARAM(native_command_queue_impl),
//				std::vector<RHI_COMMAND_BUFFER*>* command_buffer_list) {
//
//					rhi_command_buffer_record(&command_buffer, [&](RHI_VOID_PTR UNUSED_PARAM(native_command_buffer_impl)) {
//
//						auto vertices_ptr = &vertices[0];
//									
//						// cpu bridge buffer uploading
//						{
//							RHI_BUFFER_DESC shared_buffer_desc;
//							shared_buffer_desc.device = &dev;
//							shared_buffer_desc.length = vb_desc.length;
//							shared_buffer_desc.memory_type = buffer_memory_type_shared_rw;
//							shared_buffer_desc.type = buffer_type_raw;
//							shared_buffer_desc.mips = 1;
//							shared_vertex_buffer.reset(rhi_buffers_create_raw(&shared_buffer_desc));
//							rhi_buffers_map_write(shared_vertex_buffer.get(), vertices_ptr, 0, shared_buffer_desc.length);
//							rhi_buffers_gpu_upload(&command_buffer, shared_vertex_buffer.get(), vertex_buffer.get());
//						}
//
//						// cpu bridge buffer uploading
//						{
//							RHI_BUFFER_DESC shared_buffer_desc;
//							shared_buffer_desc.device = &dev;
//							shared_buffer_desc.length = ib_desc.length;
//							shared_buffer_desc.memory_type = buffer_memory_type_shared_rw;
//							shared_buffer_desc.type = buffer_type_raw;
//							shared_buffer_desc.mips = 1;
//							shared_index_buffer.reset(rhi_buffers_create_raw(&shared_buffer_desc));
//							rhi_buffers_map_write(shared_index_buffer.get(), &indices[0], 0, shared_buffer_desc.length);
//							rhi_buffers_gpu_upload(&command_buffer, shared_index_buffer.get(), index_buffer.get());
//						}
//
//						RHI_RT_BVH_DESC blas_desc;
//						blas_desc.device = &dev;
//						blas_desc.command_buffer = &command_buffer;
//						RHI_BUFFER* vb = vertex_buffer.get();
//						RHI_BUFFER* ib = index_buffer.get();
//						blas_desc.vertex_buffer = &vb;
//						blas_desc.index_buffer = &ib;
//						blas_desc.count = 1;
//						bvh.reset(rhi_rt_bvh_create(&blas_desc));
//
//						float* matrices[] = { reinterpret_cast<float*>(&rotation_matrix) };
//
//
//						RHI_RT_BVH_GEOMETRY_INSTANCE_DESC inst_desc;
//
//						RHI_RT_BVH_GEOMETRY_DESC tlas_desc;
//						tlas_desc.device = &dev;
//						tlas_desc.command_buffer = &command_buffer;
//						tlas_desc.instance_info = &inst_desc;
//						inst_desc.parent_bvh = bvh.get();
//						inst_desc.transforms = &matrices[0];
//						inst_desc.transforms_count = 1;
//						tlas_desc.read_only = false;
//						tlas_desc.instance_info_count = 1;
//						tlas_desc.total_instances = 1;
//						bvh_instances.reset(rhi_rt_bvh_build_geometry_instances(&tlas_desc));
//					});
//					command_buffer_list->push_back(&command_buffer);
//				});
//
//			
//			// view BVH (GPU read only)
//			RHI_VIEW_DESC bvh_instances_view_desc;
//			bvh_instances_view_desc.device = &dev;
//			bvh_instances_view_desc.buffer = bvh_instances.get();
//			bvh_instances_view_desc.type = shader_view_type_bvh_buffer;
//
//			RHI_MEMORY_DESCRIPTOR_SLOT* slot = nullptr;
//			resources_memory_descriptor.next_descriptor_ptr(&slot);
//			ASSERT_PTR(slot);
//			bvh_descriptor_slot.reset(slot);
//
//			bvh_instances_view_desc.memory_descriptor = bvh_descriptor_slot.get();
//			bvh_instances_view.reset(rhi_buffers_create_view(&bvh_instances_view_desc));
//
//			// view render_target (GPU read write)
//			RHI_VIEW_DESC rt_instances_view_desc;
//			rt_instances_view_desc.format = tx_desc.format;
//			rt_instances_view_desc.device = &dev;
//			rt_instances_view_desc.buffer = dynamic_cast<RHI_BUFFER*>(render_target.get());
//			rt_instances_view_desc.type = shader_view_type_rw_texture_buffer;
//
//			slot = nullptr;
//			resources_memory_descriptor.next_descriptor_ptr(&slot, 1);
//			ASSERT_PTR(slot);
//			render_target_descriptor_slot.reset(slot);
//
//			rt_instances_view_desc.memory_descriptor = render_target_descriptor_slot.get();
//			render_target_view.reset(rhi_buffers_create_view(&rt_instances_view_desc));
//
//			// view camera (constant buffer)
//			RHI_VIEW_DESC cb_instances_view_desc;
//			cb_instances_view_desc.format = tx_desc.format;
//			cb_instances_view_desc.device = &dev;
//			cb_instances_view_desc.buffer = static_cast<RHI_BUFFER*>(shared_camera_constant_buffer.get());
//			cb_instances_view_desc.type = shader_view_type_constant_buffer;
//
//			slot = nullptr;
//			resources_memory_descriptor.next_descriptor_ptr(&slot, 2);
//			ASSERT_PTR(slot);
//			camera_constant_buffer_descriptor_slot.reset(slot);
//
//			cb_instances_view_desc.memory_descriptor = camera_constant_buffer_descriptor_slot.get();
//			camera_constant_buffer_view.reset(rhi_buffers_create_view(&cb_instances_view_desc));
//
//			// create render pass
//			RHI_RENDER_PASS_DESC render_pass_desc;
//			render_pass_desc.device = &dev;
//			rt_render_pass.reset(rhi_render_pass_create(&render_pass_desc));
//			rt_render_pass->render_target_view = render_target_view.get();
//		}	
//		, [&](RHI_RENDER_PASS& render_pass) {
//			// before draw
//
//			rt_render_pass->pipeline = pipeline.get();
//			
//			float dt = get_delta_time();
//			rotation_matrix = rotate_triangle(dt);
//			float* matrices[] = { reinterpret_cast<float*>(&rotation_matrix) };
//			RHI_RT_BVH_GEOMETRY_INSTANCE_DESC inst_desc;
//			RHI_RT_BVH_GEOMETRY_DESC tlas_desc;
//			tlas_desc.device = device_ptr;
//			tlas_desc.command_buffer = command_buffer_ptr;
//			tlas_desc.instance_info = &inst_desc;
//			inst_desc.parent_bvh = bvh.get();
//			inst_desc.transforms = &matrices[0];
//			inst_desc.transforms_count = 1;
//			tlas_desc.instance_info_count = 1;
//			tlas_desc.read_only = false;
//			tlas_desc.total_instances = 1;
//			rhi_rt_bvh_update_geometry_instances(&tlas_desc, bvh_instances.get());
//
//			rhi_render_pass_execute_rt_mode(rt_render_pass.get(), command_buffer_ptr, [&] {
//
//				rhi_command_buffer_ray_trace(
//					command_buffer_ptr,
//					render_target.get(),
//					sbt.get());
//				});
//
//		}
//		, [&] (RHI_DEVICE& dev, RHI_RENDER_PASS& render_pass, RHI_COMMAND_BUFFER& command_buffer) {
//
//			
//			
//
//			//rhi_rt_bvh_update_geometry_instances(&tlas_desc, bvh_instances.get());
//
//			// draw
//			memcpy(camera_constant_buffer_ptr, &camera, sizeof(CameraCBRT));
//
//			
//			rhi_command_buffer_copy_texture(&command_buffer, dynamic_cast<RHI_TEXTURE_2D*>(render_pass.render_target_view->buffer.get()), render_target.get());
//		}
//		, [&](RHI_RENDER_PASS& UNUSED_PARAM(render_pass), RHI_SWAP_CHAIN& UNUSED_PARAM(swap_chain), RHI_COMMAND_BUFFER& UNUSED_PARAM(command_buffer)) {
//
//			// on_before_present
//			
//		},
//		[&](RHI_DEVICE& UNUSED_PARAM(device))
//			{
//				// if (on_end)
//				//	on_end(device);
//
//				rhi_buffers_map_close(shared_camera_constant_buffer.get(), 0, sizeof(CameraCBRT));
//			},
//		[&](RHI_DEVICE_DESC& device_desc) 
//			{
//				// on_configure_device
//				device_desc.features |= device_features_raytracing;
//				device_desc.shader_model = hlsl_shader_model_6_8;
//			}
//		);
//}
//
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