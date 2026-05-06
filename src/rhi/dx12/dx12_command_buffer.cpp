#include "dx12_command_buffer.hpp"
#include "dx12_command_queue.hpp"

std::unique_ptr<RHI_COMMAND_BUFFER> dx12_command_buffer_create(const RHI_COMMAND_BUFFER_DESC& desc) {

	// For simplicity, we will create a command allocator and a command list
	ID3D12Device* i_device = static_cast<ID3D12Device*>(desc.device.get());
	if (!i_device) {
		throw std::exception("Invalid device for command buffer creation");
	}
	// Create command allocator
	ID3D12CommandAllocator* i_cmd_allocator = nullptr;
	HRESULT hr = i_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&i_cmd_allocator));
	if (FAILED(hr) || !i_cmd_allocator) {
		throw std::exception("Failed to create D3D12 command allocator");
	}
	// Create command list
	ID3D12GraphicsCommandList* i_cmd_list = nullptr;
	hr = i_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, i_cmd_allocator, nullptr, IID_PPV_ARGS(&i_cmd_list));
	if (FAILED(hr) || !i_cmd_list) {
		if (i_cmd_allocator) i_cmd_allocator->Release();
		throw std::exception("Failed to create D3D12 command list");
	}
	// Close the command list as it is created in an open state
	i_cmd_list->Close();
	// Wrap the command list in a RHI_OBJECT
	return std::make_unique<DX_COMMAND_BUFFER>(i_cmd_list, i_cmd_allocator);
}

void dx12_command_buffer_record(RHI_COMMAND_BUFFER& command_buffer,
								fptr_command_buffer_on_record callback) {

	auto& cmd_buffer_impl = reinterpret_cast<DX_COMMAND_BUFFER&>(command_buffer);
	ID3D12GraphicsCommandList* i_cmd_list = static_cast<ID3D12GraphicsCommandList*>(cmd_buffer_impl);
	ID3D12CommandAllocator* i_cmd_alloc = static_cast<ID3D12CommandAllocator*>(cmd_buffer_impl);
	i_cmd_alloc->Reset();
	i_cmd_list->Reset(i_cmd_alloc, nullptr);

	callback(static_cast<RHI_VOID_PTR>(i_cmd_list));

	i_cmd_list->Close();
}