#include "dx12_index_buffer.hpp"
#include "dx12_buffers.hpp"

std::unique_ptr<RHI_INDEX_BUFFER> dx12_index_buffer_create(const RHI_INDEX_BUFFER_DESC& desc) {

    // overwrite desc to match must have index buffer requeriments
	RHI_INDEX_BUFFER_DESC ib_desc_mutable = const_cast<RHI_INDEX_BUFFER_DESC&>(desc);
	ib_desc_mutable.memory_type = buffer_memory_type_gpu_only;
    ib_desc_mutable.default_state = resource_state_index_buffer;
    ib_desc_mutable.type = buffer_type_raw;
    auto ib_impl = dx12_buffers_create_raw(ib_desc_mutable);
    ID3D12Resource* i_ib = static_cast<DX_BUFFER&>(*ib_impl);
    i_ib->AddRef();
    return std::make_unique<DX_INDEX_BUFFER>(i_ib, ib_impl->get_default_state(),
        ib_impl->get_format(), ib_impl->get_length(), desc.stride);
}