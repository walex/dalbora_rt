#include "test_api.hpp"
#define TINYDDSLOADER_IMPLEMENTATION
#include "tinyddsloader.h"

#ifdef TEST_RASTER_TEXTURED_TRIANGLE

void copy_bc1_image_data_with_mips(tinyddsloader::DDSFile & dds, const RHI_TEXTURE_MIPS* const mips,
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

void copy_bc1_image_data(tinyddsloader::DDSFile& dds, RHI_TEXTURE_2D& texture, RHI_VOID_PTR buff_ptr) {

    copy_bc1_image_data_with_mips(dds, texture.mip_maps,
        texture.mip_maps_count, buff_ptr);
}

resource_format dxgi_to_resource(tinyddsloader::DDSFile::DXGIFormat fmt)
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

struct Vertex
{
    float x, y, z;
    float u, v;
};

static Vertex vertices[] =
{
    // position  // uv
    {0.0f, 0.5f, 0.0f, 0.5f, 0.0f},
    {0.5f, -0.5f, 0.0f, 1.0f, 1.0f},
    {-0.5f, -0.5f, 0.0f, 0.0f, 1.0f},
};

void test_raster_textured_triangle(fptr_test_on_init UNUSED_PARAM(on_init),
                                   fptr_test_on_draw UNUSED_PARAM(on_draw),
                                   fptr_test_on_end UNUSED_PARAM(on_end),
                                   fptr_test_on_layout UNUSED_PARAM(on_layout),
                                   fptr_test_on_configure_device on_configure_device)
{
    
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
    

    test_raster_triangle(
        [&](RHI_DEVICE& device, RHI_COMMAND_QUEUE &command_queue,
            RHI_COMMAND_BUFFER &command_buffer, RHI_SWAP_CHAIN& UNUSED_PARAM(swap_chain))
        {
            // create sampler
            RHI_RT_SAMPLER_DESC sampler_desc;
            sampler_desc.device = &device;
            sampler.reset(rhi_sampler_create(&sampler_desc));
            
            // create texture
            RHI_TEXTURE_2D_DESC texture_desc;
            texture_desc.device = &device;
            texture_desc.memory_type = buffer_memory_type_gpu_only;
            texture_desc.type = buffer_type_image_2d;
            texture_desc.format = dxgi_to_resource(dds.GetFormat());
            texture_desc.width = static_cast<size_t>(dds.GetWidth());
            texture_desc.height = static_cast<size_t>(dds.GetHeight());
            texture_desc.is_cube_map = dds.IsCubemap();
            texture_desc.depth = static_cast<size_t>(dds.GetDepth());
            texture_desc.dims = static_cast<size_t>(dds.GetTextureDimension()) - 1;
            texture_desc.mips = static_cast<size_t>(dds.GetMipCount());
            texture.reset(rhi_texture_2d_create(&texture_desc));

            // create texture view            
            RHI_VIEW_DESC tex_view_desc;
            tex_view_desc.device = &device;
            tex_view_desc.buffer = dynamic_cast<RHI_BUFFER*>(texture.get());
            tex_view_desc.type = resource_type_texture_2d_read_only;
            tex_view_desc.format = texture_desc.format;
            tex_view_desc.mip_maps_count = texture->mip_maps_count;
            tex_view_desc.slot_id = 100;
            texture_view.reset(rhi_buffers_create_view(&tex_view_desc));

            // upload buffers
            rhi_command_queue_execute(&command_queue, true,
                [&](RHI_VOID_PTR UNUSED_PARAM(native_command_queue_impl),
                    std::vector<RHI_COMMAND_BUFFER*>* const command_buffer_list)
                {
                    rhi_command_buffer_record(&command_buffer,
                        [&](RHI_VOID_PTR UNUSED_PARAM(native_command_buffer_impl))
                        {
                            // cpu bridge buffer uploading
                            RHI_BUFFER_DESC shared_buffer_desc;
                            shared_buffer_desc.device = &device;
                            shared_buffer_desc.length = texture->hw_length;
                            shared_buffer_desc.memory_type = buffer_memory_type_shared_rw;
                            shared_buffer_desc.type = buffer_type_raw;
                            shared_buffer_desc.format = texture->hw_format;
                            shared_buffer_desc.mips = texture->mip_maps_count;
                            shared_texture_buffer.reset(rhi_buffers_create_raw(&shared_buffer_desc));
                            RHI_VOID_PTR buff_ptr = rhi_buffers_map_open(shared_texture_buffer.get(), 0, texture->hw_length);
                            
                            copy_bc1_image_data(dds, *texture.get(), buff_ptr);
                           
                            rhi_buffers_map_close(shared_texture_buffer.get(), 0, texture->hw_length);
                            rhi_texture_2d_gpu_upload(&command_buffer, shared_texture_buffer.get(), texture.get());
                        });

                    command_buffer_list->push_back(&command_buffer);
                });
        },
        [&](RHI_DEVICE& UNUSED_PARAM(device), RHI_RENDER_PASS &UNUSED_PARAM(render_pass), RHI_COMMAND_BUFFER &UNUSED_PARAM(command_buffer)) {
           // rhi_command_buffer_reset_resource_state(command_buffer, *texture);
        },
        [&](RHI_DEVICE & UNUSED_PARAM(device)) {},
        [&](RHI_PIPELINE_LAYOUT_DESC& layout, std::vector<RHI_INPUT_LAYOUT_DESC> &input_layouts, std::string &vertex_shader_path,
            std::string &pixel_shader_path, size_t &vertex_size, void **vertices_ptr)
        {
            // on_layout
            
            // descriptors
            RHI_SHADER_DESCRIPTOR_DESC& s_desc = layout.descriptors[layout.descriptor_count++];
            s_desc.resource_type = resource_type_shader;
            s_desc.shader_register_start = 0;
            s_desc.shader_register_max = 100;

            RHI_SHADER_DESCRIPTOR_DESC& sm_desc = layout.descriptors[layout.descriptor_count++];
            sm_desc.resource_type = resource_type_sampler;
            sm_desc.shader_register_start = 0;
            sm_desc.shader_register_max = 1;

            // define input layout
            RHI_INPUT_LAYOUT_DESC& desc_pos = input_layouts.emplace_back();
            strcpy_s(desc_pos.name, "POSITION");
            desc_pos.format = resource_format_float3;
            desc_pos.offset = 0;

            RHI_INPUT_LAYOUT_DESC& desc_tx = input_layouts.emplace_back();
            strcpy_s(desc_tx.name, "TEXCOORD");
            desc_tx.format = resource_format_float2;
            desc_tx.offset = 12;

            vertex_size = sizeof(Vertex);
            *vertices_ptr = &vertices[0];

            //   paths
            const std::filesystem::path shaders_path = get_executable_folder("shaders");
            const std::filesystem::path vertex_shader_file = shaders_path / "textured_triangle.hlsl";
            const std::filesystem::path pixel_shader_file = shaders_path / "textured_triangle.hlsl";
            vertex_shader_path = vertex_shader_file.string();
            pixel_shader_path = pixel_shader_file.string();
        },
        [&](RHI_DEVICE_DESC& desc) {
            if (on_configure_device)
                on_configure_device(desc);
            desc.features |= device_features_enable_texture_sampling;
        });
}

void test_raster_textured_triangle_obj(RhiUnitTestCallbacks* UNUSED_PARAM(callbacks)) {
    
    const std::filesystem::path res_path = get_executable_folder("test_3d_models");
    const std::filesystem::path texture_file = res_path / "test_texture.dds";
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

    RhiSampler sampler;
    RhiTexture texture;
    RhiView texture_view;
    RhiSharedBuffer texture_buffer;

    RhiUnitTestCallbacks unit_test_callbacks;
    unit_test_callbacks.on_device_config = ([&](__int64& features_flags) {

        features_flags |= device_features_enable_texture_sampling;
    });
    unit_test_callbacks.on_init = ([&](RhiUnitTest& unit_test) {

        RhiDevice& device = unit_test.device;
        RhiGraphicsCommandQueue& command_queue = unit_test.command_queue;
        RhiCommandBuffer& command_buffer = unit_test.command_buffer;
        RhiPipelineLayout& pipeline_layout = unit_test.pipeline_layout;
        RhiRasterPipeline& pipeline = unit_test.raster_pipeline;

        sampler.create(device);
        texture.create(device,
            dxgi_to_resource(dds.GetFormat()),
            static_cast<size_t>(dds.GetWidth()),
            static_cast<size_t>(dds.GetHeight()),
            dds.IsCubemap(),
            static_cast<size_t>(dds.GetDepth()),
            static_cast<size_t>(dds.GetTextureDimension()) - 1,
            static_cast<size_t>(dds.GetMipCount()));
        texture_view = texture.new_read_only_view(device);

        // add layout descriptor for samplers
        pipeline_layout.add_sampler_buffer_descriptors(0, 1);

        // add input descriptor
        pipeline.add_input_descriptor("TEXCOORD", 12, resource_format_float2);

        // copy texture data        
        texture_buffer.create(device, texture.get_hw_length());
        auto map_info = texture_buffer.map(0, texture.get_hw_length());
        size_t mip_count;
        const RHI_TEXTURE_MIPS* const mips = texture.get_mips(mip_count);
        copy_bc1_image_data_with_mips(dds, mips, mip_count, map_info.get_data());
        texture_buffer.unmap(map_info);

        // upload texture buffer
        // upload vertices e indices data to gpu only memory
        command_queue.sync_exec([&](RhiCommandQueueBufferList& list) {

            command_buffer.record([&] {

                texture.upload(command_buffer, texture_buffer);
            });

            list.add_command_buffer(command_buffer);
        });

        // set shader file
		std::filesystem::path shaders_path = get_executable_folder("shaders");
        unit_test.vertex_shader_file = (shaders_path / "textured_triangle.hlsl").string();
        unit_test.pixel_shader_file = (shaders_path / "textured_triangle.hlsl").string();

        // updata vertex and index buffer
        unit_test.vertices.resize(sizeof(vertices));
        memcpy(unit_test.vertices.data(), &vertices[0], sizeof(vertices));
        unit_test.vertices_stride = sizeof(vertices[0]);
    });

    unit_test_callbacks.on_draw = ([&](RhiUnitTest& UNUSED_PARAM(unit_test)) {

    
    });

    unit_test_callbacks.on_end = ([&](RhiUnitTest& UNUSED_PARAM(unit_test)) {


    });

    test_raster_triangle_obj(&unit_test_callbacks);
}

#endif