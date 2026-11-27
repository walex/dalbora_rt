#include "rhi.hpp"
#include "test_api_helpers.hpp"
#include "tiny_dds_loader.hpp"

static constexpr float image_aspect = 800.0f / 600.0f;
static constexpr float x = 0.5f;


static VertexUV vertices[] =
{
	// position  // uv
	{ 0.0f, 0.5f, 0.0f, 0.5f, 0.0f },
	{ 0.5f, -0.5f, 0.0f, 1.0f, 1.0f },
	{ -0.5f, -0.5f, 0.0f, 0.0f, 1.0f },
};

static constexpr unsigned int vertex_count = sizeof(vertices) / sizeof(VertexUV);

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
void test_raster_textured_triangle_obj() {

	constexpr size_t MAX_COMMAND_ALLOCATORS = 8;

	RhiShaderProgramHLSL vertex_shader;
	RhiShaderProgramHLSL pixel_shader;
	std::vector<uint8_t> bytecode_vs;
	std::vector<uint8_t> bytecode_ps;
	RhiGPUBuffer vertex_buffer;
	RhiGPUBuffer index_buffer;
	RhiDepthBuffer depth_buffer;
	RhiSharedBuffer camera_transforms;
	RhiSharedBuffer object_transforms;
	RhiView camera_transform_view;
	RhiView object_transform_view;
	RhiView depth_buffer_view;
	std::unique_ptr<RhiSharedBufferMap> camera_constant_buffer_map;
	std::unique_ptr<RhiSharedBufferMap> object_constant_buffer_map;

	CameraCB camera;
	ObjectCB triangle_transforms;
	get_transforms(image_aspect, triangle_transforms.world, camera.view, camera.projection);


	RhiWindow window;
	RHI_WINDOW_CALLBACKS window_callbacks;
	RhiDevice device;
	RhiGraphicsCommandQueue command_queue;
	RhiCommandBuffer command_buffer;
	RhiSwapChain swap_chain;
	RhiRasterRenderPass render_pass;
	RhiPipelineLayout pipeline_layout;
	RhiRasterPipeline pipeline;
	RHI_VIEWPORT vp;

	RhiSampler sampler;
    RhiTexture texture;
    RhiView texture_view;
    RhiSharedBuffer texture_buffer;

	window_callbacks.on_init = [&](RHI_WINDOW* hwindow) {

		RHI_DEVICE_DESC device_desc;
		device_desc.adapter_id = -1;
		device_desc.shader_model = hlsl_shader_model_6_8;
		device_desc.app_instance = rhi_get_app_instance();
		device_desc.graphics_queue_count = 1;
		device_desc.compute_queue_count = 1;
		device_desc.copy_queue_count = 1;
		device_desc.features = device_features_enable_texture_sampling;
		device.create(device_desc);

		memory_descriptors_init(device);

		RhiCommandAllocatorPool::Initialize(device, MAX_COMMAND_ALLOCATORS, queue_type_graphics);
		RhiCommandAllocatorPool::Initialize(device, MAX_COMMAND_ALLOCATORS, queue_type_compute);
		RhiCommandAllocatorPool::Initialize(device, MAX_COMMAND_ALLOCATORS, queue_type_copy);

		command_queue.create(device);
		command_buffer.create(device, command_queue);
		swap_chain.create(window, device, command_queue);
		swap_chain.create_views(device, get_rtv_memory_descriptors());
		render_pass.create(device);

		vp.x = 0;
		vp.y = 0;
		vp.width = swap_chain.get_width();
		vp.height = swap_chain.get_height();
		vp.min_z = 0.0f;
		vp.max_z = 1.0f;

		constexpr size_t constant_shader_registers_count = 2;
		constexpr size_t read_only_shader_registers_count = 0;
		constexpr size_t rw_shader_registers_count = 0;
		constexpr size_t resources_count = constant_shader_registers_count + read_only_shader_registers_count + rw_shader_registers_count;
		constexpr size_t sampler_shaders_registers_count = 1;

		std::vector<size_t> slot_group_start_indices = { constant_shader_registers_count,
			read_only_shader_registers_count, rw_shader_registers_count };
		buffers_memory_table = std::make_unique<RhiMemoryDescriptors>(device, memory_descriptor_type_buffer,
			resources_count, slot_group_start_indices);
		command_buffer.set_buffers_memory_descriptor(*buffers_memory_table);

		RhiSharedBuffer shared_vertex_buffer;
		RhiSharedBuffer shared_index_buffer;

		// setup shaders
		const std::filesystem::path shaders_path = get_executable_folder("shaders");
		const std::filesystem::path shader_file = shaders_path / "simple_triangle.hlsl";
		const std::string vertex_shader_file = shader_file.string();
		const std::string pixel_shader_file = shader_file.string();

		size_t indices_size = sizeof(indices);
		size_t indices_stride = sizeof(uint16_t);

		// add input descriptors
		pipeline.add_input_descriptor("POSITION", 0, resource_format_float3);
		pipeline.add_input_descriptor("TEXCOORD", 12, resource_format_float2);

		// save geometry buffers
		size_t vertices_size(sizeof(vertices));
		size_t vertices_stride = sizeof(vertices[0]);

		// setup pipeline layout
		// add layout descriptors ( order mathers )

		// add layout descriptor for texture samplin
		pipeline_layout.set_shader_descriptors(shader_view_type_constant_buffer, 0, constant_shader_registers_count);
		pipeline_layout.set_shader_descriptors(shader_view_type_read_only_texture_buffer, 0, read_only_shader_registers_count);
		pipeline_layout.set_shader_sampler_descriptor_offset(0, sampler_shaders_registers_count);

		// compile shaders
		{
			const std::string vs_entry_point = "VSMain";
			const std::string ps_entry_point = "PSMain";

			RhiShaderProgramHLSL vertex_shader_tmp;
			RhiShaderProgramHLSL pixel_shader_tmp;
			byte_code_format bc_format = byte_code_format_input_hlsl;
			if (rhi_get_device_type() == rhi_device_type_vulkan) {
				int flag = static_cast<int>(bc_format) | static_cast<int>(byte_code_format_output_spirv);
				bc_format = static_cast<byte_code_format>(flag);
			}
			vertex_shader_tmp.create(vertex_shader_file,
				vs_entry_point, "vs_6_0", bc_format);
			pixel_shader_tmp.create(pixel_shader_file,
				ps_entry_point, "ps_6_0", bc_format);

			vertex_shader_tmp.to_binary_file("C:\\Users\\wadrw.MSI\\Documents\\temp\\simple_triangle_vs.cso");
			pixel_shader_tmp.to_binary_file("C:\\Users\\wadrw.MSI\\Documents\\temp\\simple_triangle_ps.cso");

			vertex_shader.from_binary_file("C:\\Users\\wadrw.MSI\\Documents\\temp\\simple_triangle_vs.cso", vs_entry_point);
			pixel_shader.from_binary_file("C:\\Users\\wadrw.MSI\\Documents\\temp\\simple_triangle_ps.cso", ps_entry_point);

			// delete temporary shader files
			std::filesystem::remove("C:\\Users\\wadrw.MSI\\Documents\\temp\\simple_triangle_vs.cso");
			std::filesystem::remove("C:\\Users\\wadrw.MSI\\Documents\\temp\\simple_triangle_ps.cso");

		}


		// setup pipeline
		RhiRasterPipelineShaderPrograms shader_programs;
		shader_programs.vertex_shader = &vertex_shader;
		shader_programs.vertex_stride = vertices_stride;
		shader_programs.index_stride = indices_stride;
		shader_programs.pixel_shader = &pixel_shader;

		// create pipeline layout
		pipeline_layout.create(device, primitive_topology_triangle,
			swap_chain.get_format(), resource_format_d24_norm_s8_uint);

		// create pipeline
		pipeline.create(device, pipeline_layout,
			shader_programs, *get_buffers_memory_descriptors(),
			static_cast<RHI_COMMAND_BUFFER*>(command_buffer)->sampler_memory_descriptor.get());

		// copy vertices to cpu visible memory
		vertex_buffer.create(device, vertices_size,
			vertices_stride, resource_format_float3);
		shared_vertex_buffer.create(device, vertices_size);
		{
			auto v_map_info = shared_vertex_buffer.map(0, vertices_size);
			memcpy(v_map_info.get_data(), vertices,
				v_map_info.get_length());
		}

		// copy indices to cpu visible memory
		index_buffer.create(device, indices_size,
			indices_stride, resource_format_uint16);
		shared_index_buffer.create(device, indices_size);
		{
			auto i_map_info = shared_index_buffer.map(0, indices_size);
			memcpy(i_map_info.get_data(), indices,
				i_map_info.get_length());
		}

		// upload vertices e indices data to gpu only memory
		command_queue.sync_exec([&](RhiCommandQueueBufferList& list) {

			command_buffer.record([&] {

				vertex_buffer.upload(command_buffer, shared_vertex_buffer);
				index_buffer.upload(command_buffer, shared_index_buffer);
				});

			list.add_command_buffer(command_buffer);
			});

		// create depth buffers
		depth_buffer.create(device, window.get_width(),
			window.get_height(), resource_format_d24_norm_s8_uint);

		// create camera transform buffer
		camera_transforms.create(device, sizeof(CameraCB));

		// create triangle transform buffer
		object_transforms.create(device, sizeof(ObjectCB));

		// views
		depth_buffer_view = depth_buffer.new_view(device, shader_view_type_depth_stencil_target,
			get_dsv_memory_descriptors().next_descriptor_ptr());

		// add views for transform buffers for shader visibility
		// creation order is related with shader constant buffer registers ids
		camera_transform_view = camera_transforms.new_view(device, shader_view_type_constant_buffer,
			get_buffers_memory_descriptors().next_descriptor_ptr());		// cb reg 0
		object_transform_view = object_transforms.new_view(device, shader_view_type_constant_buffer,
			get_buffers_memory_descriptors().next_descriptor_ptr());	// cb reg 1

		// map constant buffers
		camera_constant_buffer_map = std::make_unique<RhiSharedBufferMap>(camera_transforms, 0,
			sizeof(CameraCB));
		object_constant_buffer_map = std::make_unique<RhiSharedBufferMap>(object_transforms, 0,
			sizeof(ObjectCB));

		// set depth buffer to render pass
		render_pass.set_depth_buffer(depth_buffer_view);
		// set pipeline to render pass
		render_pass.set_pipeline(pipeline);

		sampler.create(device, get_samplers_memory_descriptors().next_descriptor_ptr());
        
		const std::filesystem::path texture_folder = get_executable_folder("test_3d_models");
		const std::filesystem::path texture_file = texture_folder / "test_texture.dds";
		std::string texture_path = texture_file.string();
		tinyddsloader::DDSFile dds;
		auto ret = dds.Load(texture_path.c_str());
		if (tinyddsloader::Result::Success != ret)
		{
			throw std::exception("Failed to load texture");
		}
		if (dds.GetTextureDimension() !=
			tinyddsloader::DDSFile::TextureDimension::Texture2D)
		{
			throw std::exception("Failed is not a 2D texture");
		}

		texture.create(device,
            dxgi_to_resource(dds.GetFormat()),
            static_cast<size_t>(dds.GetWidth()),
            static_cast<size_t>(dds.GetHeight()),
            dds.IsCubemap(),
            static_cast<size_t>(dds.GetDepth()),
            static_cast<size_t>(dds.GetTextureDimension()) - 1,
            static_cast<size_t>(dds.GetMipCount()));
        texture_view = texture.new_read_only_view(device, get_buffers_memory_descriptors().next_descriptor_ptr(1));

        // copy texture data        
        texture_buffer.create(device, texture.get_hw_length());
        {
            auto map_info = texture_buffer.map(0, texture.get_hw_length());
            size_t mip_count = texture.get_mips().size();
            const RHI_TEXTURE_MIPS* const mips = texture.get_mips().data();
            copy_bc1_image_data_with_mips(dds, mips, mip_count, map_info.get_data());
        }
        // upload texture buffer
        // upload vertices e indices data to gpu only memory
        command_queue.sync_exec([&](RhiCommandQueueBufferList& list) {

            command_buffer.record([&] {

                texture.upload(command_buffer, texture_buffer);
            });

            list.add_command_buffer(command_buffer);
        });

		};

	window_callbacks.on_idle = [&](RHI_WINDOW* UNUSED_PARAM(hwindow)) {

		float dt = get_delta_time();
		triangle_transforms.world = rotate_triangle(dt);

		// upload shaders constants
		memcpy(camera_constant_buffer_map->get_data(), &camera,
			sizeof(CameraCB));
		memcpy(object_constant_buffer_map->get_data(), &triangle_transforms,
			sizeof(ObjectCB));

		command_queue.sync_exec([&](RhiCommandQueueBufferList& list) {

			command_buffer.record([&] {

				RhiView render_target_view = swap_chain.get_next_render_target();
				render_pass.set_view_port(vp);
				render_pass.set_render_target(render_target_view);
				render_pass.render(command_buffer, [&](RhiCommandBuffer& UNUSED_PARAM(command_buffer)) {

					command_buffer.draw_triangle_list(vertex_buffer, &index_buffer);
				});

					list.add_command_buffer(command_buffer);
			});			
		});
		swap_chain.present();
	};

	window_callbacks.on_end = [&](RHI_WINDOW* UNUSED_PARAM(hwindow)) {

		};


	window.create("test", 800, 600, false, window_callbacks);
	window.mainLoop();

}