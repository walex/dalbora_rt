#include "test_api.hpp"
#include "rhi.hpp"

#ifdef TEST_RASTER_TRIANGLE

void test_raster_triangle(fptr_test_on_init on_init,
						  fptr_test_on_draw on_draw,
						  fptr_test_on_end on_end,
						  fptr_test_on_layout on_layout)
{

	std::unique_ptr<RHI_RASTER_PIPELINE> pipeline;
	std::unique_ptr<RHI_BUFFER> depth_buffer;
	std::unique_ptr<RHI_VIEW> depth_buffer_view;
	std::unique_ptr<RHI_BUFFER> vertex_buffer;
	std::unique_ptr<RHI_BUFFER> index_buffer;
	std::unique_ptr<RHI_PIPELINE_LAYOUT> pipeline_layout;
	std::unique_ptr<RHI_BUFFER> shared_camera_constant_buffer;
	std::unique_ptr<RHI_BUFFER> shared_object_constant_buffer;
	std::unique_ptr<RHI_VIEW> camera_constant_buffer_view;
	std::unique_ptr<RHI_VIEW> object_constant_buffer_view;
	std::vector<RHI_BUFFER*> constants_buffer_array;
	std::unique_ptr<RHI_COMMAND_QUEUE> copy_command_queue;
	std::unique_ptr<RHI_COMMAND_BUFFER> copy_command_buffer;
	RHI_VOID_PTR camera_constant_buffer_ptr;
	RHI_VOID_PTR object_constant_buffer_ptr;

	float aspect = 800.0f / 600.0f;
	float x = 0.5f;

	CameraCB camera;
	ObjectCB triangle_transforms;

	get_transforms(triangle_transforms.world, camera.view, camera.projection);

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
			0, 1, 2};
	constexpr unsigned int index_count = sizeof(indices) / sizeof(uint16_t);

	const std::filesystem::path shaders_folder(R"(C:\Users\wadrw\Documents\develop\projects\personal\rtx\dalbora_rt\src\tests)");

	test_swap_chain([&](RHI_DEVICE &device, RHI_COMMAND_QUEUE &command_queue,
						RHI_COMMAND_BUFFER &command_buffer, RHI_SWAP_CHAIN& swap_chain)
					{
			
			std::unique_ptr<RHI_COMPILED_SHADER_BUFFER> vertex_shader;
			std::unique_ptr<RHI_COMPILED_SHADER_BUFFER> pixel_shader;
			std::unique_ptr<RHI_BUFFER> shared_vertex_buffer;
			std::unique_ptr<RHI_BUFFER> shared_index_buffer;

			// create layout for pipeline
			RHI_PIPELINE_LAYOUT_DESC pl_desc;
			pl_desc.device = &device;
			pl_desc.shader_type = shader_type_undef;

			// add constant buffer descriptors for camera and object transforms;
			RHI_DESCRIPTOR_DESC& cb_desc = pl_desc.descriptors[pl_desc.descriptor_count++];
			cb_desc.resource_type = resource_type_constant_buffer;
			cb_desc.register_start = 0; // ie: b0 in hlsl
			cb_desc.register_count = 2; // ie: b0, b1 in hlsl
			
			// create shaders layouts
			std::vector<RHI_INPUT_LAYOUT_DESC> input_layouts;

			std::string vs_file = "simple_triangle.hlsl";
			std::string ps_file = "simple_triangle.hlsl";
			size_t vertex_size = sizeof(Vertex);
			void* vertices_ptr = &vertices[0];

			RHI_INPUT_LAYOUT_DESC& desc_pos = input_layouts.emplace_back();
			strcpy_s(desc_pos.name, "POSITION");
			desc_pos.format = resource_format_float3;
			desc_pos.offset = 0;

			//if (on_layout) {
			//	on_layout(pl_desc.descriptors, input_layouts, 
			//		vs_file, ps_file, 
			//		vertex_size, &vertices_ptr);
			//}
			// 
			
			// create pipeline layout
			pipeline_layout.reset(rhi_pipeline_layout_create(&pl_desc));
			
			// compile shaders
			rhi_shaders_compiler_set_folder(shaders_folder.string().c_str());
			vertex_shader.reset(rhi_shaders_compiler_compile(vs_file.c_str(), "VSMain", "vs_6_0"));
			pixel_shader.reset(rhi_shaders_compiler_compile(ps_file.c_str(), "PSMain", "ps_6_0"));
			
			// create geometry buffers
			RHI_VERTEX_BUFFER_DESC vb_desc;
			vb_desc.device = &device;
			vb_desc.count = vertex_count;
			vb_desc.length = vertex_count * vertex_size;
			vb_desc.stride = vertex_size;
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
			shared_camera_buffer_desc.length = sizeof(CameraCB);
			shared_camera_buffer_desc.memory_type = buffer_memory_type_shared_rw;
			shared_camera_buffer_desc.type = buffer_type_raw;
			shared_camera_buffer_desc.mips = 1;
			shared_camera_constant_buffer.reset(rhi_buffers_create_constant(&shared_camera_buffer_desc));
			
			// camera constant buffer view
			RHI_VIEW_DESC camera_cb_view_desc;
			camera_cb_view_desc.device = &device;
			camera_cb_view_desc.buffer = shared_camera_constant_buffer.get();
			camera_cb_view_desc.type = resource_type_constant_buffer;
			camera_constant_buffer_view.reset(rhi_buffers_create_view(&camera_cb_view_desc));
			
			// create shared memory for object transforms
			RHI_BUFFER_DESC shared_object_buffer_desc;
			shared_object_buffer_desc.device = &device;
			shared_object_buffer_desc.length = sizeof(ObjectCB);
			shared_object_buffer_desc.memory_type = buffer_memory_type_shared_rw;
			shared_object_buffer_desc.type = buffer_type_raw;
			shared_object_buffer_desc.mips = 1;
			shared_object_constant_buffer.reset(rhi_buffers_create_constant(&shared_object_buffer_desc));

			// object constant buffer view
			RHI_VIEW_DESC object_cb_view_desc;
			object_cb_view_desc.device = &device;
			object_cb_view_desc.buffer = shared_object_constant_buffer.get();
			object_cb_view_desc.type = resource_type_constant_buffer;
			object_constant_buffer_view.reset(rhi_buffers_create_view(&object_cb_view_desc));

			camera_constant_buffer_ptr = rhi_buffers_map_open(shared_camera_constant_buffer.get(), 0, sizeof(CameraCB));
			object_constant_buffer_ptr = rhi_buffers_map_open(shared_object_constant_buffer.get(), 0, sizeof(ObjectCB));

			constants_buffer_array = {shared_camera_constant_buffer.get(), shared_object_constant_buffer.get()};

			// create copy command queue ad buffer
			RHI_COMMAND_QUEUE_DESC queue_desc; 
			queue_desc.device = &device;
			copy_command_queue.reset(rhi_command_queue_create_for_copy(&queue_desc));

			RHI_COMMAND_BUFFER_DESC command_buffer_desc;
			command_buffer_desc.device = &device;
			command_buffer_desc.command_queue = copy_command_queue.get();
			copy_command_buffer.reset(rhi_command_buffer_create_for_copy(&command_buffer_desc));

			// upload buffers
			rhi_command_queue_execute(copy_command_queue.get(), true,
				[&](RHI_VOID_PTR UNUSED_PARAM(native_command_queue_impl),
					std::vector<RHI_COMMAND_BUFFER *>* command_buffer_list) {

				rhi_command_buffer_record(copy_command_buffer.get(),
				[&](RHI_VOID_PTR UNUSED_PARAM(native_command_buffer_impl)) {

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
						rhi_buffers_gpu_upload(copy_command_buffer.get(), shared_vertex_buffer.get(), vertex_buffer.get());
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
						rhi_buffers_gpu_upload(copy_command_buffer.get(), shared_index_buffer.get(), index_buffer.get());
					}

				});

				command_buffer_list->push_back(copy_command_buffer.get()); 
			});

			// create depth buffer
			RHI_TEXTURE_2D_DESC db_desc;
			db_desc.device = &device;
			db_desc.width = 800;
			db_desc.height = 600;
			db_desc.format = resource_format_d24_norm_s8_uint;
			db_desc.type = buffer_type_depth_stencil;
			depth_buffer.reset(rhi_buffers_create_depth(&db_desc));

			// depth view
			RHI_VIEW_DESC db_view_desc;
			db_view_desc.device = &device;
			db_view_desc.buffer = depth_buffer.get();
			db_view_desc.type = resource_type_depth_stencil_target;
			db_view_desc.format = db_desc.format;
			depth_buffer_view.reset(rhi_buffers_create_view(&db_view_desc));
			
			// create pipeline
			RHI_RASTER_PIPELINE_DESC pipe_desc;
			pipe_desc.device = &device;
			pipe_desc.layout = pipeline_layout.get();
			pipe_desc.input_layouts_desc_count = input_layouts.size();
			memcpy(&pipe_desc.input_layouts_desc[0], input_layouts.data(), input_layouts.size() * sizeof(RHI_INPUT_LAYOUT_DESC));
			pipe_desc.vertex_shader = vertex_shader.get();
			pipe_desc.pixel_shader = pixel_shader.get();
			pipe_desc.topology = primitive_topology_triangle;
			pipe_desc.surface_format = resource_format_R8G8B8A8_norm;
			pipe_desc.depth_buffer_format = resource_format_d24_norm_s8_uint;
			pipeline.reset(rhi_raster_pipeline_create(&pipe_desc));

			// on init
			if (on_init)
				on_init(device, command_queue, command_buffer, swap_chain); 
			},

			[&](RHI_RENDER_PASS &render_pass)
			{
				// on before draw
				render_pass.depth_buffer_view = depth_buffer_view.get();
				render_pass.pipeline = pipeline.get();
			},
			[&](RHI_DEVICE& device, RHI_RENDER_PASS& render_pass,
				RHI_COMMAND_BUFFER &command_buffer)
			{
				// on draw

				float dt = get_delta_time();
				triangle_transforms.world = rotate_triangle(dt);

				// upload shaders constants
				memcpy(camera_constant_buffer_ptr, &camera, sizeof(CameraCB));
				memcpy(object_constant_buffer_ptr, &triangle_transforms, sizeof(ObjectCB));

				if (on_draw)
					on_draw(device, render_pass, command_buffer);
				rhi_command_buffer_draw_triangle_list(&command_buffer, vertex_buffer.get(), index_buffer.get());
			},
			[&](RHI_RENDER_PASS &UNUSED_PARAM(render_pass), RHI_SWAP_CHAIN& (swap_chain), RHI_COMMAND_BUFFER& UNUSED_PARAM(command_buffer))
			{
				// on after draw
			},
			[&](RHI_DEVICE &UNUSED_PARAM(device))
			{
				// if (on_end)
				//	on_end(device);

				// on end
				rhi_buffers_map_close(shared_camera_constant_buffer.get(), 0, sizeof(CameraCB));
				rhi_buffers_map_close(shared_object_constant_buffer.get(), 0, sizeof(ObjectCB));
			});
	return;
}

#endif