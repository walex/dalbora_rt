#include "dx12_fence.hpp"

std::unique_ptr<HAL_OBJECT> dx12_create_fence(const HAL_FENCE_DESC& desc) {

	DX_DEVICE_HANDLE* device_handle = static_cast<DX_DEVICE_HANDLE*>(desc.device->get_native_impl().get());
	ID3D12Device5* device = *device_handle;
	ID3D12Fence* fence = nullptr;
	HRESULT hr = device->CreateFence(desc.initial_value, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	if (FAILED(hr)) {
		throw std::runtime_error("Failed to create fence");
	}
	return std::make_unique<HAL_OBJECT>(new DX_FENCE_HANDLE(fence));
}
