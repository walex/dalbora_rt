#include "rhi.hpp"
#include "test_api_helpers.hpp"
#define TINYDDSLOADER_IMPLEMENTATION
#include "tinyddsloader.h"

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

static void copy_bc1_image_data_with_mips(tinyddsloader::DDSFile & dds, const RHI_TEXTURE_MIPS* const mips,
        size_t mip_count, RHI_VOID_PTR buff_ptr) {
    
    for (size_t mipIndex = 0;
        mipIndex < mip_count;
        ++mipIndex)
    {
        const tinyddsloader::DDSFile::ImageData* image =
            dds.GetImageData(static_cast<uint32_t>(mipIndex), 0);

        const uint8_t* src =
            reinterpret_cast<const uint8_t*>(
                image->m_mem);

        auto& mip =
            mips[mipIndex];

        uint8_t* dst =
            reinterpret_cast<uint8_t*>(buff_ptr) +
            mip.offset;

        // BC1 = bloques 4x4 de 8 bytes

        UINT blockWidth =
            (image->m_width + 3) / 4;

        UINT srcRowSize =
            blockWidth * 8;

        // Numero de filas de bloques BC1

        UINT numRows =
            (image->m_height + 3) / 4;

        // mip.pitch viene de GetCopyableFootprints
        // y YA incluye alineamiento DX12

        for (UINT row = 0; row < numRows; ++row)
        {
            memcpy(
                dst + row * mip.pitch,
                src + row * srcRowSize,
                srcRowSize);
        }
    }
}

static void copy_bc1_image_data(tinyddsloader::DDSFile& dds, RHI_TEXTURE_2D& texture, RHI_VOID_PTR buff_ptr) {

    copy_bc1_image_data_with_mips(dds, texture.mip_maps.data(),
        texture.mip_maps.size(), buff_ptr);
}

static resource_format dxgi_to_resource(tinyddsloader::DDSFile::DXGIFormat fmt)
{

    static constexpr tinyddsloader::DDSFile::DXGIFormat dxgi_resource_format_type[] = {
        tinyddsloader::DDSFile::DXGIFormat::Unknown,            // resource_format_none
        tinyddsloader::DDSFile::DXGIFormat::R8_UInt,           // resource_format_uint8
        tinyddsloader::DDSFile::DXGIFormat::R16_UInt,           // resource_format_uint16
        tinyddsloader::DDSFile::DXGIFormat::R32_UInt,           // resource_format_uint32
        tinyddsloader::DDSFile::DXGIFormat::R8G8B8A8_UNorm,     // resource_format_R8G8B8A8
        tinyddsloader::DDSFile::DXGIFormat::R32_Float,          // resource_format_float
        tinyddsloader::DDSFile::DXGIFormat::R32G32_Float,       // resource_format_float2
        tinyddsloader::DDSFile::DXGIFormat::R32G32B32_Float,    // resource_format_float3
        tinyddsloader::DDSFile::DXGIFormat::R32G32B32A32_Float, // resource_format_float4
        tinyddsloader::DDSFile::DXGIFormat::Unknown,            // resource_format_d32_float_s8_uint
        tinyddsloader::DDSFile::DXGIFormat::Unknown,            // resource_format_d24_norm_s8_uint
        tinyddsloader::DDSFile::DXGIFormat::D32_Float,          // resource_format_32_float
        tinyddsloader::DDSFile::DXGIFormat::D16_UNorm,          // resource_format_d16_norm,
        tinyddsloader::DDSFile::DXGIFormat::BC1_UNorm           // resource_format_bc1_norm
    };

    std::span<const tinyddsloader::DDSFile::DXGIFormat> s(dxgi_resource_format_type);
    auto it = std::find(s.begin(), s.end(), fmt);
    if (it == s.end())
    {
        throw std::exception("texture format no supported");
    }
    return static_cast<resource_format>(std::distance(s.begin(), it));
}

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

void test_raster_textured_triangle() {

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

		std::unique_ptr<RHI_MEMORY_DESCRIPTOR_SLOT> camera_descriptor_slot;
		std::unique_ptr<RHI_MEMORY_DESCRIPTOR_SLOT> object_descriptor_slot;
		std::unique_ptr<RHI_MEMORY_DESCRIPTOR_SLOT> depth_descriptor_slot;

		const std::filesystem::path shaders_path = get_executable_folder("test_3d_models");
		const std::filesystem::path shader_file = shaders_path / "test_texture.dds";
		std::string texture_path = shader_file.string();
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
    
		std::unique_ptr<RHI_TEXTURE_2D> texture;
		std::unique_ptr<RHI_VIEW> texture_view;
		std::unique_ptr<RHI_SAMPLER> sampler;
		std::unique_ptr<RHI_BUFFER> shared_texture_buffer;
		std::unique_ptr<RHI_MEMORY_DESCRIPTOR_SLOT> sampler_memory_descriptor_slot;
		std::unique_ptr<RHI_MEMORY_DESCRIPTOR_SLOT> tview_memory_descriptor_slot;

		CameraCB camera;
		ObjectCB triangle_transforms;
		get_transforms(image_aspect, triangle_transforms.world, camera.view, camera.projection);

		const std::filesystem::path shaders_folder = get_executable_folder("shaders");

		std::shared_ptr<RHI_WINDOW_CALLBACKS> callbacks = std::make_shared<RHI_WINDOW_CALLBACKS>();

		callbacks.get()->on_init = ([&](RHI_WINDOW* const window) {

			RHI_DEVICE_DESC device_desc;
			device_desc.adapter_id = -1;
			device_desc.features = device_features_enable_texture_sampling;
			device_desc.app_instance = rhi_get_app_instance();
			device_desc.graphics_queue_count = 1;
			device_desc.compute_queue_count = 1;
			device_desc.copy_queue_count = 1;
			device.reset(rhi_create_device(&device_desc));

			memory_descriptors_init(device.get());

			RhiMemoryDescriptors& resources_memory_descriptor = get_buffers_memory_descriptors();
			RhiMemoryDescriptors& samplers_memory_descriptor = get_samplers_memory_descriptors();

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
			command_buffer->buffer_memory_descriptor = resources_memory_descriptor;

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


			RhiMemoryDescriptors& memory_descriptor = get_buffers_memory_descriptors();

			std::unique_ptr<RHI_COMPILED_SHADER_BUFFER> vertex_shader;
			std::unique_ptr<RHI_COMPILED_SHADER_BUFFER> pixel_shader;
			std::unique_ptr<RHI_BUFFER> shared_vertex_buffer;
			std::unique_ptr<RHI_BUFFER> shared_index_buffer;

			// create layout for pipeline
			RHI_PIPELINE_LAYOUT_DESC pl_desc;
			pl_desc.device = device.get();

			// add constant buffer descriptors for camera and object transforms;
			RHI_SHADER_DESCRIPTOR_DESC& cb_desc = pl_desc.descriptors[pl_desc.descriptor_count++];
			cb_desc.shader_view_type = shader_view_type_constant_buffer;
			cb_desc.shader_register_start = 0; // ie: b0 in hlsl
			cb_desc.shader_register_max = 5;

            RHI_SHADER_DESCRIPTOR_DESC& s_desc = pl_desc.descriptors[pl_desc.descriptor_count++];
            s_desc.shader_view_type = shader_view_type_read_only_texture_buffer;
            s_desc.shader_register_start = 0;
            s_desc.shader_register_max = 5;

            RHI_SHADER_DESCRIPTOR_DESC& sm_desc = pl_desc.descriptors[pl_desc.descriptor_count++];
            sm_desc.shader_view_type = shader_view_type_sampler;
            sm_desc.shader_register_start = 0;
            sm_desc.shader_register_max = 1;
 
			// create shaders layouts
			std::vector<RHI_INPUT_LAYOUT_DESC> input_layouts;

			std::string vs_file;
			std::string ps_file;
			size_t vertex_size;
			void* vertices_ptr;

			vs_file = "textured_triangle.hlsl";
			ps_file = "textured_triangle.hlsl";
			vertex_size = sizeof(VertexUV);
			vertices_ptr = &vertices[0];


			// define input layout
			RHI_INPUT_LAYOUT_DESC& desc_pos = input_layouts.emplace_back();
			strcpy_s(desc_pos.name, "POSITION");
			desc_pos.format = resource_format_float3;
			desc_pos.offset = 0;

			RHI_INPUT_LAYOUT_DESC& desc_tx = input_layouts.emplace_back();
			strcpy_s(desc_tx.name, "TEXCOORD");
			desc_tx.format = resource_format_float2;
			desc_tx.offset = 12;

			vs_file = std::filesystem::path(shaders_folder / vs_file).string();
			ps_file = std::filesystem::path(shaders_folder / ps_file).string();

			const char* vs_entry_point = "VSMain";
			const char* ps_entry_point = "PSMain";

			// compile shaders
			int flags = byte_code_format_input_hlsl;
			if (rhi_get_device_type() == rhi_device_type_vulkan) {
				flags |= byte_code_format_output_spirv;
			}
			vertex_shader.reset(rhi_hlsl_compiler_compile(vs_file.c_str(), vs_entry_point, "vs_6_0", shaders_folder.string().c_str(), static_cast<byte_code_format>(flags)));
			pixel_shader.reset(rhi_hlsl_compiler_compile(ps_file.c_str(), ps_entry_point, "ps_6_0", shaders_folder.string().c_str(), static_cast<byte_code_format>(flags)));

			// create pipeline layout
			pipeline_layout.reset(rhi_pipeline_layout_create(&pl_desc));

			// create geometry buffers
			RHI_VERTEX_BUFFER_DESC vb_desc;
			vb_desc.device = device.get();
			vb_desc.count = vertex_count;
			vb_desc.length = vertex_count * vertex_size;
			vb_desc.stride = vertex_size;
			vb_desc.format = resource_format_float3;
			vb_desc.memory_type = buffer_memory_type_gpu_read_only;
			vb_desc.type = buffer_type_raw;
			vertex_buffer.reset(rhi_buffers_create_vertices(&vb_desc));

			RHI_INDEX_BUFFER_DESC ib_desc;
			ib_desc.device = device.get();
			ib_desc.count = index_count;
			ib_desc.length = sizeof(uint16_t) * index_count;
			ib_desc.stride = sizeof(uint16_t);
			ib_desc.format = resource_format_uint16;
			ib_desc.memory_type = buffer_memory_type_gpu_read_only;
			ib_desc.type = buffer_type_raw;
			index_buffer.reset(rhi_buffers_create_indices(&ib_desc));

			// create shared memory for camera transforms
			RHI_BUFFER_DESC shared_camera_buffer_desc;
			shared_camera_buffer_desc.device = device.get();
			shared_camera_buffer_desc.length = sizeof(CameraCB);
			shared_camera_buffer_desc.memory_type = buffer_memory_type_shared_rw;
			shared_camera_buffer_desc.type = buffer_type_raw;
			shared_camera_buffer_desc.mips = 1;
			shared_camera_constant_buffer.reset(rhi_buffers_create_constant(&shared_camera_buffer_desc));

			// camera constant buffer view
			RHI_VIEW_DESC camera_cb_view_desc;
			camera_cb_view_desc.device = device.get();
			camera_cb_view_desc.buffer = shared_camera_constant_buffer.get();
			camera_cb_view_desc.type = shader_view_type_constant_buffer;

			RHI_MEMORY_DESCRIPTOR_SLOT* slot = nullptr;
			memory_descriptor.next_descriptor_ptr(&slot);
			ASSERT_PTR(slot);
			camera_descriptor_slot.reset(slot);

			camera_cb_view_desc.memory_descriptor = camera_descriptor_slot.get();
			camera_constant_buffer_view.reset(rhi_buffers_create_view(&camera_cb_view_desc));

			// create shared memory for object transforms
			RHI_BUFFER_DESC shared_object_buffer_desc;
			shared_object_buffer_desc.device = device.get();
			shared_object_buffer_desc.length = sizeof(ObjectCB);
			shared_object_buffer_desc.memory_type = buffer_memory_type_shared_rw;
			shared_object_buffer_desc.type = buffer_type_raw;
			shared_object_buffer_desc.mips = 1;
			shared_object_constant_buffer.reset(rhi_buffers_create_constant(&shared_object_buffer_desc));

			// object constant buffer view
			RHI_VIEW_DESC object_cb_view_desc;
			object_cb_view_desc.device = device.get();
			object_cb_view_desc.buffer = shared_object_constant_buffer.get();
			object_cb_view_desc.type = shader_view_type_constant_buffer;

			slot = nullptr;
			memory_descriptor.next_descriptor_ptr(&slot);
			ASSERT_PTR(slot);
			object_descriptor_slot.reset(slot);

			object_cb_view_desc.memory_descriptor = object_descriptor_slot.get();
			object_constant_buffer_view.reset(rhi_buffers_create_view(&object_cb_view_desc));

			camera_constant_buffer_ptr = rhi_buffers_map_open(shared_camera_constant_buffer.get(), 0, sizeof(CameraCB));
			object_constant_buffer_ptr = rhi_buffers_map_open(shared_object_constant_buffer.get(), 0, sizeof(ObjectCB));

			constants_buffer_array = { shared_camera_constant_buffer.get(), shared_object_constant_buffer.get() };

			// create copy command queue and buffer
			queue_desc.type = queue_type_copy;
			copy_command_queue.reset(rhi_command_queue_create(&queue_desc));

			command_buffer_desc.command_queue = copy_command_queue.get();
			command_buffer_desc.command_allocator = rhi_command_allocator_pool_acquire(command_allocator_pool[copy_command_queue->type]);
			copy_command_buffer.reset(rhi_command_buffer_create(&command_buffer_desc));


			// create depth buffer
			RHI_TEXTURE_2D_DESC db_desc;
			db_desc.device = device.get();
			db_desc.width = swap_chain->buffer_width;
			db_desc.height = swap_chain->buffer_height;
			db_desc.format = resource_format_d24_norm_s8_uint;
			db_desc.type = buffer_type_depth_stencil;
			depth_buffer.reset(rhi_buffers_create_depth(&db_desc));

			// depth view
			RHI_VIEW_DESC db_view_desc;
			db_view_desc.device = device.get();
			db_view_desc.buffer = depth_buffer.get();
			db_view_desc.type = shader_view_type_depth_stencil_target;
			db_view_desc.format = db_desc.format;

			slot = nullptr;
			get_dsv_memory_descriptors().next_descriptor_ptr(&slot);
			ASSERT_PTR(slot);
			depth_descriptor_slot.reset(slot);

			db_view_desc.memory_descriptor = depth_descriptor_slot.get();
			depth_buffer_view.reset(rhi_buffers_create_view(&db_view_desc));

			// create pipeline
			RHI_RASTER_PIPELINE_DESC pipe_desc;
			pipe_desc.device = device.get();
			pipe_desc.layout = pipeline_layout.get();
			pipe_desc.input_layouts_desc_count = input_layouts.size();
			memcpy(&pipe_desc.input_layouts_desc[0], input_layouts.data(), input_layouts.size() * sizeof(RHI_INPUT_LAYOUT_DESC));
			pipe_desc.vertex_shader = vertex_shader.get();
			pipe_desc.pixel_shader = pixel_shader.get();
			pipe_desc.topology = primitive_topology_triangle;
			pipe_desc.format = resource_format_R8G8B8A8_norm;
			pipe_desc.depth_buffer_format = resource_format_d24_norm_s8_uint;
			pipe_desc.vertex_stride = vertex_size;
			pipe_desc.index_stride = sizeof(uint16_t);
			pipe_desc.resources_memory_descriptor = memory_descriptor;
			pipe_desc.sampler_memory_descriptor = samplers_memory_descriptor;
			strcpy(pipe_desc.vertex_shader_entry_point, vs_entry_point);
			strcpy(pipe_desc.pixel_shader_entry_point, ps_entry_point);
			pipeline.reset(rhi_raster_pipeline_create(&pipe_desc));

			// upload buffers		
			rhi_command_queue_execute(copy_command_queue.get(), true,
				[&](RHI_VOID_PTR UNUSED_PARAM(native_command_queue_impl),
					std::vector<RHI_COMMAND_BUFFER*>* command_buffer_list) {

						rhi_command_buffer_record(copy_command_buffer.get(),
							[&](RHI_VOID_PTR UNUSED_PARAM(native_command_buffer_impl)) {

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
									rhi_buffers_gpu_upload(copy_command_buffer.get(), shared_vertex_buffer.get(), vertex_buffer.get());
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
									rhi_buffers_gpu_upload(copy_command_buffer.get(), shared_index_buffer.get(), index_buffer.get());
								}

							});

						command_buffer_list->push_back(copy_command_buffer.get());
				});

				command_buffer->sampler_memory_descriptor = samplers_memory_descriptor;

				// create sampler
				RHI_SAMPLER_DESC sampler_desc;
				sampler_desc.device = device.get();

				slot = nullptr;
				samplers_memory_descriptor.next_descriptor_ptr(&slot);
				ASSERT_PTR(slot);
				sampler_memory_descriptor_slot.reset(slot);

				sampler_desc.memory_descriptor_slot = sampler_memory_descriptor_slot.get();
				sampler.reset(rhi_sampler_create(&sampler_desc));
            
				// create texture
				RHI_TEXTURE_2D_DESC texture_desc;
				texture_desc.device = device.get();
				texture_desc.memory_type = buffer_memory_type_gpu_read_only;
				texture_desc.type = buffer_type_image_2d;
				texture_desc.format = dxgi_to_resource(dds.GetFormat());
				texture_desc.width = static_cast<size_t>(dds.GetWidth());
				texture_desc.height = static_cast<size_t>(dds.GetHeight());
				texture_desc.is_cube_map = dds.IsCubemap();
				texture_desc.depth = static_cast<size_t>(dds.GetDepth());
				texture_desc.dims = static_cast<size_t>(dds.GetTextureDimension()) - 1;
				texture_desc.mips = static_cast<size_t>(dds.GetMipCount());
				texture_desc.resource_flags = resource_flags_texture_sampling;
				texture.reset(rhi_texture_2d_create(&texture_desc));

				// create texture view            
				RHI_VIEW_DESC tex_view_desc;
				tex_view_desc.device = device.get();
				tex_view_desc.buffer = dynamic_cast<RHI_BUFFER*>(texture.get());
				tex_view_desc.type = shader_view_type_read_only_texture_buffer;
				tex_view_desc.format = texture_desc.format;
				tex_view_desc.mip_maps_count = texture->mip_maps.size();

				slot = nullptr;
				resources_memory_descriptor.next_descriptor_ptr(&slot, 1);
				ASSERT_PTR(slot);
				tview_memory_descriptor_slot.reset(slot);

				tex_view_desc.memory_descriptor = tview_memory_descriptor_slot.get();
				texture_view.reset(rhi_buffers_create_view(&tex_view_desc));

				// upload buffers
				rhi_command_queue_execute(command_queue.get(), true,
					[&](RHI_VOID_PTR UNUSED_PARAM(native_command_queue_impl),
						std::vector<RHI_COMMAND_BUFFER*>* const command_buffer_list)
					{
						rhi_command_buffer_record(command_buffer.get(),
							[&](RHI_VOID_PTR UNUSED_PARAM(native_command_buffer_impl))
							{
								// cpu bridge buffer uploading
								RHI_BUFFER_DESC shared_buffer_desc;
								shared_buffer_desc.device = device.get();
								shared_buffer_desc.length = texture->hw_length;
								shared_buffer_desc.memory_type = buffer_memory_type_shared_rw;
								shared_buffer_desc.type = buffer_type_raw;
								shared_buffer_desc.format = texture->hw_format;
								shared_buffer_desc.mips = texture->mip_maps.size();
								shared_texture_buffer.reset(rhi_buffers_create_raw(&shared_buffer_desc));
								RHI_VOID_PTR buff_ptr = rhi_buffers_map_open(shared_texture_buffer.get(), 0, texture->hw_length);
                            
								copy_bc1_image_data(dds, *texture.get(), buff_ptr);
                           
								rhi_buffers_map_close(shared_texture_buffer.get(), 0, texture->hw_length);
								rhi_texture_2d_gpu_upload(command_buffer.get(), shared_texture_buffer.get(), texture.get());
							});

						command_buffer_list->push_back(command_buffer.get());
					});

			});

		callbacks.get()->on_idle = ([&](const RHI_WINDOW* UNUSED_PARAM(window)) {

			swap_chain->current_image_index = rhi_swap_chain_get_current_buffer_id(swap_chain.get());
			command_buffer->rtv_memory_descriptor = get_rtv_memory_descriptors();
			command_buffer->dsv_memory_descriptor = get_dsv_memory_descriptors();
			render_pass->render_target_view = views[swap_chain->current_image_index].get();
			render_pass->depth_buffer_view = depth_buffer_view.get();
			render_pass->pipeline = pipeline.get();

			rhi_command_queue_execute(command_queue.get(), true, [&](
				RHI_VOID_PTR UNUSED_PARAM(native_command_queue_impl),
				std::vector<RHI_COMMAND_BUFFER*>* const command_buffer_list) {

					rhi_command_buffer_record(command_buffer.get(),
						[&](RHI_VOID_PTR UNUSED_PARAM(native_command_buffer_impl)) {

							// begin pass
							rhi_render_pass_execute_raster_mode(render_pass.get(), command_buffer.get(), [&] {

								render_pass->depth_buffer_view = depth_buffer_view.get();
								render_pass->pipeline = pipeline.get();
								float dt = get_delta_time();
								triangle_transforms.world = rotate_triangle(dt);

								// upload shaders constants
								memcpy(camera_constant_buffer_ptr, &camera, sizeof(CameraCB));
								memcpy(object_constant_buffer_ptr, &triangle_transforms, sizeof(ObjectCB));

								rhi_command_buffer_draw_triangle_list(command_buffer.get(), vertex_buffer.get(), index_buffer.get());

								});
						});
					command_buffer_list->push_back(command_buffer.get());
				});

			// present
			rhi_swap_chain_present(swap_chain.get());
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

//void test_raster_textured_triangle_obj(RhiUnitTestCallbacks* UNUSED_PARAM(callbacks)) {
//    
//    const std::filesystem::path res_path = get_executable_folder("test_3d_models");
//    const std::filesystem::path texture_file = res_path / "test_texture.dds";
//    std::string texture_path = texture_file.string();
//    tinyddsloader::DDSFile dds;
//    auto ret = dds.Load(texture_path.c_str());
//    if (tinyddsloader::Result::Success != ret)
//    {
//        throw std::exception("Failed to load texture");
//    }
//    if (dds.GetTextureDimension() !=
//        tinyddsloader::DDSFile::TextureDimension::Texture2D)
//    {
//        throw std::exception("Failed is not a 2D texture");
//    }
//
//    RhiSampler sampler;
//    RhiTexture texture;
//    RhiView texture_view;
//    RhiSharedBuffer texture_buffer;
//
//    RhiUnitTestCallbacks unit_test_callbacks;
//    unit_test_callbacks.on_device_config = ([&](RHI_DEVICE_DESC& device_desc) {
//
//        device_desc.features |= device_features_enable_texture_sampling;
//    });
//    unit_test_callbacks.on_init = ([&](RhiUnitTest& unit_test) {
//
//        RhiDevice& device = unit_test.device;
//        RhiGraphicsCommandQueue& command_queue = unit_test.command_queue;
//        RhiCommandBuffer& command_buffer = unit_test.command_buffer;
//        RhiPipelineLayout& pipeline_layout = unit_test.pipeline_layout;
//        RhiRasterPipeline& pipeline = unit_test.raster_pipeline;
//
//        set_samplers_memory_table(std::make_unique<RhiMemoryTable>(device, memory_descriptor_type_sampler,
//            SAMPLER_DESCRIPTORS_COUNT));
//
//        sampler.create(device, get_samplers_memory_table().next_descriptor_ptr());
//        texture.create(device,
//            dxgi_to_resource(dds.GetFormat()),
//            static_cast<size_t>(dds.GetWidth()),
//            static_cast<size_t>(dds.GetHeight()),
//            dds.IsCubemap(),
//            static_cast<size_t>(dds.GetDepth()),
//            static_cast<size_t>(dds.GetTextureDimension()) - 1,
//            static_cast<size_t>(dds.GetMipCount()));
//        texture_view = texture.new_read_only_view(device, get_buffers_memory_descriptors().next_descriptor_ptr(1));
//
//        // add layout descriptor for texture samplin
//        pipeline_layout.set_shader_descriptors(shader_view_type_constant_buffer, 0, unit_test.constant_shader_registers_count);
//        pipeline_layout.set_shader_descriptors(shader_view_type_read_only_texture_buffer, 0, 1);
//        pipeline_layout.set_shader_sampler_descriptor_offset(0, 1);
//
//        // add input descriptors
//        pipeline.add_input_descriptor("POSITION", 0, resource_format_float3);
//        pipeline.add_input_descriptor("TEXCOORD", 12, resource_format_float2);
//
//        // copy texture data        
//        texture_buffer.create(device, texture.get_hw_length());
//        {
//            auto map_info = texture_buffer.map(0, texture.get_hw_length());
//            size_t mip_count = texture.get_mips().size();
//            const RHI_TEXTURE_MIPS* const mips = texture.get_mips().data();
//            copy_bc1_image_data_with_mips(dds, mips, mip_count, map_info.get_data());
//        }
//        // upload texture buffer
//        // upload vertices e indices data to gpu only memory
//        command_queue.sync_exec([&](RhiCommandQueueBufferList& list) {
//
//            command_buffer.record([&] {
//
//                texture.upload(command_buffer, texture_buffer);
//            });
//
//            list.add_command_buffer(command_buffer);
//        });
//
//        // set shader file
//		std::filesystem::path shaders_path = get_executable_folder("shaders");
//        unit_test.vertex_shader_file = (shaders_path / "textured_triangle.hlsl").string();
//        unit_test.pixel_shader_file = (shaders_path / "textured_triangle.hlsl").string();
//
//        // update vertex buffer
//        unit_test.vertices.resize(sizeof(vertices));
//        memcpy(unit_test.vertices.data(), &vertices[0], sizeof(vertices));
//        unit_test.vertices_stride = sizeof(vertices[0]);
//
//		command_buffer.set_sampler_memory_descriptor(get_samplers_memory_table());
//
//    });
//
//    unit_test_callbacks.on_memory_descriptor_config = ([&](size_t& constant_shader_registers_count,
//        size_t& read_only_shader_registers_count, size_t& rw_shader_registers_count) {
//
//            read_only_shader_registers_count = 1;
//            return true;
//        });
//
//    unit_test_callbacks.on_draw = ([&](RhiUnitTest& UNUSED_PARAM(unit_test)) {
//
//    
//    });
//
//    unit_test_callbacks.on_end = ([&](RhiUnitTest& UNUSED_PARAM(unit_test)) {
//
//
//    });
//
//    test_raster_triangle_obj(&unit_test_callbacks);
//}
//
