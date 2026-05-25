#include "dx12_fence.hpp"

RHI_FENCE* dx12_fence_create(const RHI_FENCE_DESC* const desc) {

	ASSERT_PTR(desc);
	ASSERT_PTR(desc->device);

	ID3D12Device* i_device = *static_cast<DX_DEVICE*>(desc->device);
	ASSERT_PTR(i_device);

	ID3D12Fence* i_fence = nullptr;
	ASSERT_SUCCESS(i_device->CreateFence(desc->initial_value,
		D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&i_fence)));
	ASSERT_PTR(i_fence);

	DX_FENCE* result = new DX_FENCE();
	result->set_handle(i_fence);
	return result;
}
