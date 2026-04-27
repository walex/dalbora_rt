#include "dx12_vertex_buffer.hpp"

std::unique_ptr<RHI_OBJECT> dx12_create_vertex_buffer(const RHI_VERTEX_BUFFER_DESC& vb_desc) {

    ID3D12Resource* vb = nullptr;

    D3D12_HEAP_PROPERTIES heapProps;
    heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProps.CreationNodeMask = 1;
    heapProps.VisibleNodeMask = 1;

    size_t buffSize = vb_desc.stride * vb_desc.count;
    D3D12_RESOURCE_DESC buffer;
    buffer.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    buffer.Alignment = 0;
    buffer.Width = (UINT)buffSize;
    buffer.Height = 1;
    buffer.DepthOrArraySize = 1;
    buffer.MipLevels = 1;
    buffer.Format = DXGI_FORMAT_UNKNOWN;
    buffer.SampleDesc.Count = 1;
    buffer.SampleDesc.Quality = 0;
    buffer.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    buffer.Flags = D3D12_RESOURCE_FLAG_NONE;

    HRESULT hr = dx_rhi_get_interface<ID3D12Device5>(*vb_desc.device)->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &buffer,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(&vb)
	);
    if (FAILED(hr) || !vb) {
        throw std::exception("Failed to create D3D12 vertex buffer");
	}
	return std::make_unique<DX_RHI_RESOURCE>(new DX_BUFFER_HANDLE(vb));
}