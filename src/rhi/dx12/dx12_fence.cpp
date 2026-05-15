#include "dx12_fence.hpp"

std::unique_ptr<RHI_FENCE> dx12_fence_create(const RHI_FENCE_DESC& desc) {

	ID3D12Fence* fence = nullptr;
	ID3D12Device* i_device = desc.device.get();
	HRESULT hr = i_device->CreateFence(desc.initial_value, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	if (FAILED(hr)) {
		throw std::runtime_error("Failed to create fence");
	}
	return std::make_unique<DX_FENCE>(fence);
}
