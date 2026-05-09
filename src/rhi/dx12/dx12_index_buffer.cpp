#include "dx12_index_buffer.hpp"
#include "dx12_buffers.hpp"

std::unique_ptr<RHI_BUFFER> dx12_index_buffer_create(const RHI_INDEX_BUFFER_DESC& desc) {

    // overwrite desc to match must have index buffer requeriments
	RHI_INDEX_BUFFER_DESC ib_desc_mutable = const_cast<RHI_INDEX_BUFFER_DESC&>(desc);
	ib_desc_mutable.memory_type = buffer_memory_type_gpu_only;
    ib_desc_mutable.base_state = resource_state_index_buffer;
    ib_desc_mutable.type = buffer_type_raw;
    return dx12_buffers_create_raw(ib_desc_mutable);
}