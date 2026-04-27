#include "dx12_texture_2d.hpp"

std::unique_ptr<RHI_OBJECT> dx12_create_texture_2d(const RHI_TEXTURE_2D_DESC& tex_desc) {
	ID3D12Resource* texture = nullptr;
	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
	heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProps.CreationNodeMask = 1;
	heapProps.VisibleNodeMask = 1;
	D3D12_RESOURCE_DESC textureDesc = {};
	textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	textureDesc.Alignment = 0;
	textureDesc.Width = (UINT)tex_desc.width;
	textureDesc.Height = (UINT)tex_desc.height;
	textureDesc.DepthOrArraySize = 1;
	textureDesc.MipLevels = 1;
	textureDesc.Format = dx12_color_format_type[(int)tex_desc.format];
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	HRESULT hr = dx_rhi_get_interface<ID3D12Device5>(*tex_desc.device)->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&textureDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&texture)
	);
	if (FAILED(hr) || !texture) {
		throw std::exception("Failed to create D3D12 2D texture");
	}
	return std::make_unique<DX_RHI_RESOURCE>(new DX_BUFFER_HANDLE(texture));
}