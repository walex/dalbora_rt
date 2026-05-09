#include "dx12_vertex_buffer.hpp"
#include "dx12_buffers.hpp"

std::unique_ptr<RHI_BUFFER> dx12_vertex_buffer_create(const RHI_VERTEX_BUFFER_DESC& desc) {

    // overwrite desc to match must have vertex buffer requeriments
    RHI_VERTEX_BUFFER_DESC vb_desc_mutable = const_cast<RHI_VERTEX_BUFFER_DESC&>(desc);
    vb_desc_mutable.memory_type = buffer_memory_type_gpu_only;
    vb_desc_mutable.base_state = resource_state_constant_buffer;
    vb_desc_mutable.type = buffer_type_raw;
    return dx12_buffers_create_raw(vb_desc_mutable);
}

