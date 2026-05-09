#include "dx12_texture_2d.hpp"

std::unique_ptr<RHI_TEXTURE_2D> dx12_texture_2d_create(const RHI_TEXTURE_2D_DESC& desc) {

	ID3D12Resource* i_texture = nullptr;
	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
	heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProps.CreationNodeMask = 1;
	heapProps.VisibleNodeMask = 1;
	D3D12_RESOURCE_DESC textureDesc = {};
	textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	textureDesc.Alignment = 0;
	textureDesc.Width = (UINT)desc.width;
	textureDesc.Height = (UINT)desc.height;
	textureDesc.DepthOrArraySize = 1;
	textureDesc.MipLevels = 1;
	textureDesc.Format = dx12_resource_format_type[(int)desc.format];
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	HRESULT hr = static_cast<ID3D12Device*>(desc.device.get())->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&textureDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&i_texture)
	);
	if (FAILED(hr) || !i_texture) {
		throw std::exception("Failed to create D3D12 2D texture");
	}
	return std::make_unique<DX_TEXTURE_2D>(i_texture, desc.base_state, 
		desc.format, desc.width, desc.height, desc.stride);
}