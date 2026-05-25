#include "test_api.hpp"
#include "rhi.hpp"
#define TINYDDSLOADER_IMPLEMENTATION
#include "tinyddsloader.h"

#ifdef TEST_RASTER_TEXTURED_TRIANGLE

void copy_bc1_image_data(tinyddsloader::DDSFile& dds, RHI_TEXTURE_2D& texture, RHI_VOID_PTR buff_ptr) {

    
    auto& mips = texture.get_mips();
    for (size_t mipIndex = 0;
        mipIndex < mips.size();
        ++mipIndex)
    {
        const tinyddsloader::DDSFile::ImageData* image =
            dds.GetImageData(mipIndex, 0);

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

resource_format dxgi_to_resource(tinyddsloader::DDSFile::DXGIFormat fmt)
{

    static constexpr tinyddsloader::DDSFile::DXGIFormat dxgi_resource_format_type[] = {
        tinyddsloader::DDSFile::DXGIFormat::Unknown,            // resource_format_none
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

void test_raster_textured_triangle(fptr_test_on_init UNUSED_PARAM(on_init),
                                   fptr_test_on_draw UNUSED_PARAM(on_draw),
                                   fptr_test_on_end UNUSED_PARAM(on_end),
                                   fptr_test_on_layout UNUSED_PARAM(on_layout))
{
    struct Vertex
    {
        float x, y, z;
        float u, v;
    };

    Vertex vertices[] =
        {
            // position  // uv
            {0.0f, 0.5f, 0.0f, 0.5f, 0.0f},
            {0.5f, -0.5f, 0.0f, 1.0f, 1.0f},
            {-0.5f, -0.5f, 0.0f, 0.0f, 1.0f},
        };

    std::string texture_path = R"(C:\Users\wadrw\Documents\develop\projects\personal\rtx\dalbora_rt\src\tests\test_texture.dds)";
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
    std::unique_ptr<RHI_SAMPLER> sampler;

    test_raster_triangle(
        [&](RHI_DEVICE &device, RHI_COMMAND_QUEUE &command_queue,
            RHI_COMMAND_BUFFER &command_buffer, RHI_SWAP_CHAIN& UNUSED_PARAM(swap_chain))
        {
            RHI_RT_SAMPLER_DESC sampler_desc(device);
            sampler_desc.resource_slot = 0;
            sampler = rhi_sampler_create(sampler_desc);

            RHI_TEXTURE_2D_DESC texture_desc(device);
            texture_desc.memory_type = buffer_memory_type_gpu_only;
            texture_desc.default_state = resource_state_shader_read;
            texture_desc.type = buffer_type_image_2d;
            texture_desc.format = dxgi_to_resource(dds.GetFormat());
            texture_desc.width = static_cast<size_t>(dds.GetWidth());
            texture_desc.height = static_cast<size_t>(dds.GetHeight());
            texture_desc.is_cube_map = dds.IsCubemap();
            texture_desc.depth = static_cast<size_t>(dds.GetDepth());
            texture_desc.dims = static_cast<size_t>(dds.GetTextureDimension()) - 1;
            texture_desc.mips = static_cast<size_t>(dds.GetMipCount());
            texture_desc.resource_slot = 2;
            // load texture data from file
            texture = rhi_texture_2d_create(texture_desc);

            // upload buffers
            rhi_command_queue_execute(command_queue, true,
                [&](RHI_VOID_PTR UNUSED_PARAM(native_command_queue_impl),
                    std::vector<RHI_COMMAND_BUFFER*>& command_buffer_list)
                {
                    rhi_command_buffer_record(command_buffer,
                        [&](RHI_VOID_PTR UNUSED_PARAM(native_command_buffer_impl))
                        {
                            // cpu bridge buffer uploading
                            RHI_BUFFER_DESC shared_buffer_desc(device);
                            shared_buffer_desc.length = texture->get_length();
                            shared_buffer_desc.memory_type = buffer_memory_type_shared_rw;
                            shared_buffer_desc.type = buffer_type_raw;
                            shared_buffer_desc.format = texture_desc.format;
                            shared_buffer_desc.default_state = resource_state_generic_read;
                            auto shared_texture_buffer = rhi_buffers_create_raw(shared_buffer_desc);
                            RHI_VOID_PTR buff_ptr = rhi_buffers_map_open(*shared_texture_buffer, 0, texture->get_length());
                            
                            copy_bc1_image_data(dds, *texture, buff_ptr);
                           
                            rhi_buffers_map_close(*shared_texture_buffer, 0, texture->get_length());
                            rhi_texture_2d_gpu_upload(command_buffer, *shared_texture_buffer, *texture);
                        });

                    command_buffer_list.push_back(&command_buffer);
                });
        },
        [&](RHI_DEVICE& device, RHI_RENDER_PASS &render_pass, RHI_COMMAND_BUFFER &command_buffer) {
           // rhi_command_buffer_reset_resource_state(command_buffer, *texture);
        },
        [&](RHI_DEVICE &device) {},
        [&](std::vector<RHI_DESCRIPTOR_DESC>& descriptors, std::vector<RHI_INPUT_LAYOUT_DESC> &input_layouts, std::string &vertex_shader_path,
            std::string &pixel_shader_path, size_t &vertex_size, void **vertices_ptr)
        {
            // on_layout
            
            // descriptors
            RHI_DESCRIPTOR_DESC s_desc;
            s_desc.resource_type = resource_type_shader;
            s_desc.pool_range_start = 0;
            s_desc.pool_range_count = 1;
            descriptors.push_back(s_desc);

            RHI_DESCRIPTOR_DESC sm_desc;
            sm_desc.resource_type = resource_type_sampler;
            sm_desc.pool_range_start = 0;
            sm_desc.pool_range_count = 1;
            descriptors.push_back(sm_desc);

            // define input layout
            input_layouts.emplace_back("POSITION", resource_format_float3, 0);
            input_layouts.emplace_back("TEXCOORD", resource_format_float2, 12);
            vertex_size = sizeof(Vertex);
            *vertices_ptr = &vertices[0];

            // shaders paths
            vertex_shader_path = R"(C:\Users\wadrw\Documents\develop\projects\personal\rtx\dalbora_rt\src\tests\textured_triangle.hlsl)";
            pixel_shader_path = R"(C:\Users\wadrw\Documents\develop\projects\personal\rtx\dalbora_rt\src\tests\textured_triangle.hlsl)";
        });
}

#endif