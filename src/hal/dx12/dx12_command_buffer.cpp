#include "dx12_command_buffer.hpp"

std::unique_ptr<HAL_OBJECT> dx12_create_command_buffer(const HAL_COMMAND_BUFFER_DESC& cb_desc) {

	// For simplicity, we will create a command allocator and a command list
	ID3D12Device5* device5 = dx_hal_get_interface<ID3D12Device5>(*cb_desc.device);
	if (!device5) {
		throw std::exception("Invalid device for command buffer creation");
	}
	// Create command allocator
	ID3D12CommandAllocator* commandAllocator = nullptr;
	HRESULT hr = device5->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
	if (FAILED(hr) || !commandAllocator) {
		throw std::exception("Failed to create D3D12 command allocator");
	}
	// Create command list
	ID3D12GraphicsCommandList* commandList = nullptr;
	hr = device5->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, nullptr, IID_PPV_ARGS(&commandList));
	if (FAILED(hr) || !commandList) {
		if (commandAllocator) commandAllocator->Release();
		throw std::exception("Failed to create D3D12 command list");
	}
	// Close the command list as it is created in an open state
	commandList->Close();
	// Wrap the command list in a HAL_OBJECT
	return std::make_unique<HAL_OBJECT>(new DX_COMMAND_BUFFER_HANDLE(commandList));
}