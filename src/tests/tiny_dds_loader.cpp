#define TINYDDSLOADER_IMPLEMENTATION
#include "tiny_dds_loader.hpp"

void copy_bc1_image_data_with_mips(tinyddsloader::DDSFile& dds, const RHI_TEXTURE_MIPS* const mips,
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

    copy_bc1_image_data_with_mips(dds, texture.mip_maps.data(),
        texture.mip_maps.size(), buff_ptr);
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