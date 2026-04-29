#ifndef __dx12_texture_2d_hpp__
#define __dx12_texture_2d_hpp__

#include "dx12_rhi.hpp"

std::unique_ptr<RHI_OBJECT> dx12_texture_2d_create(const RHI_TEXTURE_2D_DESC& tex_desc);
#endif