#ifndef __tiny_dds_loader_h__
#define __tiny_dds_loader_h__

#include "rhi.hpp"
#include "tinyddsloader.h"

void copy_bc1_image_data_with_mips(tinyddsloader::DDSFile& dds, const RHI_TEXTURE_MIPS* const mips,
    size_t mip_count, RHI_VOID_PTR buff_ptr);
void copy_bc1_image_data(tinyddsloader::DDSFile& dds, RHI_TEXTURE_2D& texture, RHI_VOID_PTR buff_ptr);
resource_format dxgi_to_resource(tinyddsloader::DDSFile::DXGIFormat fmt);

#endif