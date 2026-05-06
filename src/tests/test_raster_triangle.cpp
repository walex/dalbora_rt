#include "test_api.hpp"
#include "rhi.hpp"

void test_raster_triangle(std::function<void(RHI_DEVICE& device)> UNUSED_PARAM(on_init)
	, std::function<void(RHI_RENDER_PASS& render_pass)> UNUSED_PARAM(on_draw)
	, std::function<void(RHI_DEVICE& device)> UNUSED_PARAM(on_end)) {

	std::unique_ptr<RHI_DESCRIPTOR_POOL> shaders_descriptor_pool;
	std::unique_ptr<RHI_DESCRIPTOR_POOL> depth_buffer_descriptor_pool;
	std::unique_ptr<RHI_PIPELINE_LAYOUT> pipeline_pool;
	std::unique_ptr<RHI_BUFFER> vertex_buffer;
	std::unique_ptr<RHI_BUFFER> index_buffer;
	std::unique_ptr<RHI_SHADER_BUFFER> vertex_shader;
	std::unique_ptr<RHI_SHADER_BUFFER> pixel_shader;
	std::unique_ptr<RHI_RASTER_PIPELINE> raster_pipeline;
	std::unique_ptr<RHI_DEPTH_BUFFER> depth_buffer;

	struct Vertex
	{
		float x, y, z;
	};

	Vertex vertices[] =
	{
		{  0.0f,  0.5f, 0.0f }, // top
		{  0.5f, -0.5f, 0.0f }, // right
		{ -0.5f, -0.5f, 0.0f }  // left
	};
	constexpr unsigned int vertex_count = sizeof(vertices) / sizeof(vertices[0]);

	uint16_t indices[] =
	{
		0, 1, 2
	};
	constexpr unsigned int index_count = sizeof(indices) / sizeof(indices[0]);
	const std::filesystem::path shaders_folder(R"(C:\Users\wadrw\Documents\develop\projects\personal\rtx\dalbora_rt\src\tests)");
	
	test_swap_chain([&](RHI_DEVICE& device, RHI_COMMAND_QUEUE& command_queue, 
		RHI_COMMAND_BUFFER& command_buffer) {

		std::unique_ptr<RHI_BUFFER> cpu_vertex_buffer;
		std::unique_ptr<RHI_BUFFER> cpu_index_buffer;
		// on init

		//if (on_init)
		//	on_init(device, command_queue, command_buffer);

		// create pool for shader descriptors
		RHI_DESCRIPTOR_POOL_DESC dp_shaders_desc(device);
		dp_shaders_desc.resource_type = resource_type_generic_rw_buffer;
		dp_shaders_desc.device = std::reference_wrapper(device);
		dp_shaders_desc.slot_count = 10;
		dp_shaders_desc.shader_visibility = true;
		shaders_descriptor_pool = rhi_descriptor_pool_create(dp_shaders_desc);

		// create pool for depth stencil buffer only dx12, not used in other case ( returns null )
		// FixME: hide when support another graphics api
		RHI_DESCRIPTOR_POOL_DESC dp_depth_desc(device);
		dp_depth_desc.resource_type = resource_type_depth_stencil_target;
		dp_depth_desc.device = std::reference_wrapper(device);
		dp_depth_desc.slot_count = 1;
		depth_buffer_descriptor_pool = rhi_descriptor_pool_create(dp_depth_desc);

		// create layout for pipeline 
		RHI_PIPELINE_LAYOUT_DESC pl_desc(device);
		pl_desc.shader_type = shader_type_undef;
		
		// add constant buffer descriptor for triangle material, world view proj matrices;
		RHI_DESCRIPTOR_DESC cb_desc;
		cb_desc.resource_type = resource_type_constant_buffer;
		cb_desc.pool_range_start = 0;
		cb_desc.pool_range_count = 4;
		pl_desc.descriptors.push_back(cb_desc);

		// add texture sampler descriptor
		RHI_DESCRIPTOR_DESC s_desc;
		s_desc.resource_type = resource_type_sampler;
		s_desc.pool_range_start = 0;
		s_desc.pool_range_count = 1;
		pl_desc.descriptors.push_back(s_desc);

		// create pipeline layout
		pipeline_pool = rhi_pipeline_layout_create(pl_desc);

		// compile shaders
		rhi_shaders_compiler_set_folder(shaders_folder.string().c_str());
		vertex_shader = rhi_shaders_compiler_compile("simple_triangle.hlsl", "VSMain", "vs_6_0");
		pixel_shader = rhi_shaders_compiler_compile("simple_triangle.hlsl", "PSMain", "ps_6_0");

		// create geometry buffers
		RHI_VERTEX_BUFFER_DESC vb_desc(device);
		vb_desc.count = vertex_count;
		vb_desc.stride = sizeof(Vertex);
		vb_desc.format = resource_format_float3;
		vertex_buffer = rhi_vertex_buffer_create(vb_desc);

		RHI_INDEX_BUFFER_DESC ib_desc(device);
		ib_desc.count = index_count;
		ib_desc.stride = sizeof(uint16_t);
		ib_desc.format = resource_format_uint16;
		index_buffer = rhi_index_buffer_create(ib_desc);

		rhi_command_queue_execute(command_queue, true, [&](RHI_VOID_PTR UNUSED_PARAM(native_command_queue_impl),
			std::vector<RHI_COMMAND_BUFFER*>& command_buffer_list) {

				rhi_command_buffer_record(command_buffer, 
					[&](RHI_VOID_PTR UNUSED_PARAM(native_command_buffer_impl)) {
					
					// cpu bridge buffer
					{
						RHI_BUFFER_DESC cpu_buffer_desc(device);
						cpu_buffer_desc.width = vb_desc.count * vb_desc.stride;
						cpu_buffer_desc.height = 1;
						cpu_buffer_desc.memory_type = buffer_memory_type_cpu_to_gpu;
						cpu_vertex_buffer = rhi_buffers_create_raw(cpu_buffer_desc);
						rhi_buffers_cpu_write(*cpu_vertex_buffer, &vertices[0], 0, cpu_buffer_desc.width * cpu_buffer_desc.height);
						rhi_buffers_gpu_write(command_buffer, *cpu_vertex_buffer, *vertex_buffer);
					}

					// cpu bridge buffer
					{
						RHI_BUFFER_DESC cpu_buffer_desc(device);
						cpu_buffer_desc.width = ib_desc.count * ib_desc.stride;
						cpu_buffer_desc.height = 1;
						cpu_buffer_desc.memory_type = buffer_memory_type_cpu_to_gpu;
						cpu_index_buffer = rhi_buffers_create_raw(cpu_buffer_desc);
						rhi_buffers_cpu_write(*cpu_index_buffer, &indices[0], 0, cpu_buffer_desc.width * cpu_buffer_desc.height);
						rhi_buffers_gpu_write(command_buffer, *cpu_index_buffer, *index_buffer);
					}

					// create depth buffer
					RHI_DEPTH_BUFFER_DESC db_desc(device);
					db_desc.width = 800;
					db_desc.height = 600;
					db_desc.format = resource_format_d32_float_s8_uint;
					// only used by dx12
					db_desc.pool = depth_buffer_descriptor_pool.get();
					depth_buffer = rhi_buffers_create_depth(db_desc);
				
					});

				command_buffer_list.push_back(&command_buffer);
			});

		// create shaders layouts
		std::vector<RHI_INPUT_LAYOUT_DESC> input_layouts;
		input_layouts.emplace_back("POSITION", resource_format_float3, 0);

		// set depth_buffer in render pass
		//rhi_render_pass_set_depth_buffer();
		// create pipeline
		RHI_RASTER_PIPELINE_DESC pipe_desc(device, *pipeline_pool, input_layouts, vertex_shader.get(), pixel_shader.get());
		pipe_desc.topology = primitive_topology_triangle;
		pipe_desc.surface_format = resource_format_R8G8B8A8_norm;
		raster_pipeline = rhi_raster_pipeline_create(pipe_desc);

		},
		[&](RHI_RENDER_PASS& render_pass) {

			// on before draw

			rhi_render_pass_set_depth_buffer(render_pass, depth_buffer.get());

		},
		[&](RHI_RENDER_PASS& UNUSED_PARAM(render_pass)) {

			// on draw


		},
		[&](RHI_RENDER_PASS& UNUSED_PARAM(render_pass)) {

			// on after draw

		},
		[&](RHI_DEVICE& UNUSED_PARAM(device)) {

			//if (on_end)
			//	on_end(device);

			// on end
		});
	return;

}

