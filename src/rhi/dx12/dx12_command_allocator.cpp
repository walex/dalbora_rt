#include "dx12_command_allocator.hpp"

RHI_COMMAND_ALLOCATOR* dx12_command_allocator_create(const RHI_DEVICE* const device, const queue_type type) {

	ID3D12Device* i_device = *static_cast<const DX_DEVICE*>(device);
	ASSERT_PTR(i_device);

	D3D12_COMMAND_LIST_TYPE dx12_type = dx12_queue_type[type];

	// Create command allocator
	ID3D12CommandAllocator* i_cmd_allocator = nullptr;
	ASSERT_SUCCESS(i_device->CreateCommandAllocator(dx12_type, IID_PPV_ARGS(&i_cmd_allocator)));
	ASSERT_PTR(i_cmd_allocator);

	DX_COMMAND_ALLOCATOR* result = new DX_COMMAND_ALLOCATOR();
	result->set_handle(i_cmd_allocator);
	return result;
}
