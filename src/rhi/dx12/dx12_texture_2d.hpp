#ifndef __dx12_texture_2d_hpp__
#define __dx12_texture_2d_hpp__

#include "dx12_rhi.hpp"

std::unique_ptr<RHI_TEXTURE_2D> dx12_texture_2d_create(
    const RHI_TEXTURE_2D_DESC &tex_desc);
void dx12_texture_2d_gpu_upload(RHI_COMMAND_BUFFER& command_buffer, RHI_BUFFER& shared_buffer, RHI_TEXTURE_2D& texture);
#endif