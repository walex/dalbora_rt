#include "rhi_render_target.hpp"
#include "rhi_device.hpp"
#include "rhi_shared_buffer.hpp"
#include "rhi_command_buffer.hpp"

RhiRenderTarget::RhiRenderTarget(RHI_TEXTURE_2D* handle) : RhiTexture(handle) {}

void RhiRenderTarget::create(const RhiDevice& device, const resource_format format,
    const size_t width, const size_t height) {

    RHI_TEXTURE_2D_DESC desc;
    desc.device = device;
    desc.memory_type = buffer_memory_type_gpu_only;
    desc.type = buffer_type_image_2d;
    desc.width = width;
    desc.height = height;
    desc.format = format;
    desc.is_render_target = true;
    desc.mips = 1;
    desc.flags = resource_flags_shader_read_write;    
    this->set_handle(rhi_texture_2d_create(&desc));
}