#ifndef __dx12_texture_2d_hpp__
#define __dx12_texture_2d_hpp__

#include "dx12_rhi.hpp"

RHI_TEXTURE_2D* dx12_texture_2d_create(
    const RHI_TEXTURE_2D_DESC* const desc);
void dx12_texture_2d_gpu_upload(RHI_COMMAND_BUFFER* const command_buffer, const RHI_BUFFER* const src_buffer, RHI_TEXTURE_2D* const dest_buffer);
#endif