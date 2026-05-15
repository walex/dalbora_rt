#include "dx12_vertex_buffer.hpp"
#include "dx12_buffers.hpp"

std::unique_ptr<RHI_VERTEX_BUFFER> dx12_vertex_buffer_create(const RHI_VERTEX_BUFFER_DESC& desc) {

    // overwrite desc to match must have vertex buffer requeriments
    RHI_VERTEX_BUFFER_DESC vb_desc_mutable = const_cast<RHI_VERTEX_BUFFER_DESC&>(desc);
    vb_desc_mutable.memory_type = buffer_memory_type_gpu_only;
    vb_desc_mutable.default_state = resource_state_vertex_buffer;
    vb_desc_mutable.type = buffer_type_raw;
    auto vb_impl = dx12_buffers_create_raw(vb_desc_mutable);
    ID3D12Resource* i_vb = static_cast<DX_BUFFER&>(*vb_impl);
    i_vb->AddRef();
    return std::make_unique<DX_VERTEX_BUFFER>(i_vb, vb_impl->get_default_state(),
        vb_impl->get_format(), vb_impl->get_length(), desc.stride);
}

