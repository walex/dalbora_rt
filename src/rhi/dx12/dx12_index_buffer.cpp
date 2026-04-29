#include "dx12_index_buffer.hpp"
#include "dx12_buffers.hpp"

std::unique_ptr<RHI_OBJECT> dx12_index_buffer_create(const RHI_INDEX_BUFFER_DESC& ib_desc) {

	// overwrite the size in the buffer desc to match the index buffer size just in case
	RHI_INDEX_BUFFER_DESC& ib_desc_mutable = const_cast<RHI_INDEX_BUFFER_DESC&>(ib_desc);
	ib_desc_mutable.size = ib_desc.count * (size_t)ib_desc.format;
	ib_desc_mutable.memory_type = buffer_memory_type_gpu_only;
    ib_desc_mutable.initial_state = resource_state_constant_buffer;
    return dx12_buffers_create(ib_desc);
}

void dx12_index_buffer_update(RHI_OBJECT& index_buffer, const void* data, size_t size) {

    ID3D12Resource* ib = index_buffer.handle<DX_BUFFER_HANDLE>();

    void* mappedData = nullptr;
    D3D12_RANGE readRange = { 0, 0 }; // We do not intend to read from this resource on CPU
    HRESULT hr = ib->Map(0, &readRange, &mappedData);
    if (FAILED(hr) || !mappedData) {
        throw std::exception("Failed to map D3D12 index buffer");
    }
    memcpy(mappedData, data, size);
    ib->Unmap(0, nullptr);
}