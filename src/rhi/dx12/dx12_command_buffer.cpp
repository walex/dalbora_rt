#include "dx12_command_buffer.hpp"
#include "dx12_command_queue.hpp"

std::unique_ptr<RHI_OBJECT> dx12_command_buffer_create(const RHI_COMMAND_BUFFER_DESC& desc) {

	// For simplicity, we will create a command allocator and a command list
	ID3D12Device* device = desc.device().handle<DX_DEVICE_HANDLE>();
	if (!device) {
		throw std::exception("Invalid device for command buffer creation");
	}
	// Create command allocator
	ID3D12CommandAllocator* commandAllocator = nullptr;
	HRESULT hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
	if (FAILED(hr) || !commandAllocator) {
		throw std::exception("Failed to create D3D12 command allocator");
	}
	// Create command list
	ID3D12GraphicsCommandList* commandList = nullptr;
	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, nullptr, IID_PPV_ARGS(&commandList));
	if (FAILED(hr) || !commandList) {
		if (commandAllocator) commandAllocator->Release();
		throw std::exception("Failed to create D3D12 command list");
	}
	// Close the command list as it is created in an open state
	commandList->Close();
	// Wrap the command list in a RHI_OBJECT
	return std::make_unique<RHI_OBJECT>(new DX_COMMAND_BUFFER_HANDLE(commandList));
}