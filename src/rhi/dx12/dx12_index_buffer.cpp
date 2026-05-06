#include "dx12_index_buffer.hpp"
#include "dx12_buffers.hpp"

std::unique_ptr<RHI_BUFFER> dx12_index_buffer_create(const RHI_INDEX_BUFFER_DESC& desc) {

    // overwrite desc to match must have index buffer requeriments
	RHI_INDEX_BUFFER_DESC ib_desc_mutable = const_cast<RHI_INDEX_BUFFER_DESC&>(desc);
    ib_desc_mutable.width = desc.stride * desc.count;
    ib_desc_mutable.height = 1;
	ib_desc_mutable.memory_type = buffer_memory_type_gpu_only;
    ib_desc_mutable.initial_state = resource_state_constant_buffer;
    ib_desc_mutable.type = buffer_type_raw;
    return dx12_buffers_create_raw(ib_desc_mutable);
}

void dx12_index_buffer_update(RHI_BUFFER& index_buffer, const void* data, size_t size) {

    ID3D12Resource* ib = static_cast<ID3D12Resource*>(index_buffer);
    void* mappedData = nullptr;
    D3D12_RANGE readRange = { 0, 0 }; // We do not intend to read from this resource on CPU
    HRESULT hr = ib->Map(0, &readRange, &mappedData);
    if (FAILED(hr) || !mappedData) {
        throw std::exception("Failed to map D3D12 index buffer");
    }
    memcpy(mappedData, data, size);
    ib->Unmap(0, nullptr);
}