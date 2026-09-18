#include "rhi_texture.hpp"
#include "rhi.hpp"

RhiTexture::RhiTexture(RHI_TEXTURE_2D* handle) : RhiImpl<RHI_TEXTURE_2D>(handle) {}

void RhiTexture::create(const RhiDevice& device, const resource_format format,
    const size_t width, const size_t height,
    const bool is_cube_map, const size_t depth,
    const size_t dimension, const size_t mip_count) {

    RHI_TEXTURE_2D_DESC desc;
    desc.device = device;
    desc.memory_type = buffer_memory_type_gpu_only;
    desc.type = buffer_type_image_2d;
    desc.format = format;
    desc.width = width;
    desc.height = height;
    desc.is_cube_map = is_cube_map;
    desc.depth = depth;
    desc.dims = dimension;
    desc.mips = mip_count;
    this->set_handle(rhi_texture_2d_create(&desc));
}

RhiView RhiTexture::new_view(const RhiDevice& device, RHI_MEMORY_DESCRIPTOR_SLOT* memory_descriptor_slot) {
    return RhiTexture::new_read_only_view(device, memory_descriptor_slot);
}

RhiView RhiTexture::new_read_only_view(const RhiDevice& device, RHI_MEMORY_DESCRIPTOR_SLOT* memory_descriptor_slot) {

    //RHI_VIEW_DESC tex_view_desc;
    //tex_view_desc.device = device;
    //tex_view_desc.buffer = dynamic_cast<RHI_BUFFER*>(static_cast<RHI_TEXTURE_2D*>(*this));
    //tex_view_desc.type = shader_view_type_read_only_texture_buffer;
    //tex_view_desc.format = static_cast<RHI_TEXTURE_2D*>(*this)->hw_format;
    //tex_view_desc.mip_maps_count = static_cast<RHI_TEXTURE_2D*>(*this)->mip_maps_count;
    //tex_view_desc.memory_descriptor = memory_descriptor_slot;
    //return RhiView(rhi_buffers_create_view(&tex_view_desc));

    RhiView view;
    RhiBuffer buffer(dynamic_cast<RHI_BUFFER*>(static_cast<RHI_TEXTURE_2D*>(*this)));
    view.create(device, buffer, memory_descriptor_slot,
        shader_view_type_read_only_texture_buffer, static_cast<RHI_TEXTURE_2D*>(*this)->hw_format,
        static_cast<RHI_TEXTURE_2D*>(*this)->mip_maps_count);
    return view;
}

RhiView RhiTexture::new_rw_view(const RhiDevice& device, RHI_MEMORY_DESCRIPTOR_SLOT* memory_descriptor_slot) {
 //   RHI_VIEW_DESC tex_view_desc;
 //   tex_view_desc.device = device;
 //   tex_view_desc.buffer = dynamic_cast<RHI_BUFFER*>(static_cast<RHI_TEXTURE_2D*>(*this));
 //   tex_view_desc.type = shader_view_type_rw_texture_buffer;
 //   tex_view_desc.format = static_cast<RHI_TEXTURE_2D*>(*this)->hw_format;
 //   tex_view_desc.mip_maps_count = static_cast<RHI_TEXTURE_2D*>(*this)->mip_maps_count;
	//tex_view_desc.memory_descriptor = memory_descriptor_slot;
 //   return RhiView(rhi_buffers_create_view(&tex_view_desc));

    RhiView view;
    RhiBuffer buffer(dynamic_cast<RHI_BUFFER*>(static_cast<RHI_TEXTURE_2D*>(*this)));
    view.create(device, buffer, memory_descriptor_slot,
        shader_view_type_rw_texture_buffer, static_cast<RHI_TEXTURE_2D*>(*this)->hw_format,
        static_cast<RHI_TEXTURE_2D*>(*this)->mip_maps_count);
    return view;
}

size_t RhiTexture::get_hw_length() {
    return static_cast<RHI_TEXTURE_2D*>(*this)->hw_length;
}

const RHI_TEXTURE_MIPS* const RhiTexture::get_mips(size_t &mip_count) {

    mip_count = static_cast<RHI_TEXTURE_2D*>(*this)->mip_maps_count;
    return &static_cast<RHI_TEXTURE_2D*>(*this)->mip_maps[0];
}

void RhiTexture::upload(RhiCommandBuffer& command_buffer, RhiSharedBuffer& buffer) {
    rhi_texture_2d_gpu_upload(command_buffer, buffer, *this);
}