#include "dx12_vertex_buffer.hpp"
#include "dx12_buffers.hpp"
#include "dx12_command_buffer.hpp"
#include "dx12_resource_state.hpp"

std::unique_ptr<RHI_OBJECT> dx12_vertex_buffer_create(const RHI_VERTEX_BUFFER_DESC& vb_desc) {

    // overwrite the size in the buffer desc to match the index buffer size just in case
    RHI_VERTEX_BUFFER_DESC& vb_desc_mutable = const_cast<RHI_VERTEX_BUFFER_DESC&>(vb_desc);
    vb_desc_mutable.size = vb_desc.stride * vb_desc.count;
    vb_desc_mutable.memory_type = buffer_memory_type_gpu_only;
    vb_desc_mutable.initial_state = resource_state_constant_buffer;
    return dx12_buffers_create(vb_desc);
}

void dx12_vertex_buffer_upload(RHI_TRANSFER_BUFFER_DESC& desc) {

    dx12_buffers_upload(desc);
}

void dx12_vertex_buffer_download(RHI_TRANSFER_BUFFER_DESC& desc) {

    dx12_buffers_download_synchronized(desc);
}