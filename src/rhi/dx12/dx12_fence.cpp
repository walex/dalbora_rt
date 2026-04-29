#include "dx12_fence.hpp"

std::unique_ptr<RHI_OBJECT> dx12_fence_create(const RHI_FENCE_DESC& desc) {

	ID3D12Fence* fence = nullptr;
	ID3D12Device* device = desc.device->handle<DX_DEVICE_HANDLE>();
	HRESULT hr = device->CreateFence(desc.initial_value, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	if (FAILED(hr)) {
		throw std::runtime_error("Failed to create fence");
	}
	return std::make_unique<RHI_OBJECT>(new DX_FENCE_HANDLE(fence));
}
