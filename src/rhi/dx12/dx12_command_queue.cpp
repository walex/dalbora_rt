#include "dx12_command_queue.hpp"
#include "dx12_fence.hpp"

std::unique_ptr<RHI_OBJECT> dx12_create_command_queue(const RHI_COMMAND_QUEUE_DESC& queue_desc, queue_type type) {
	
	ID3D12Device* device = dx_rhi_get_interface<ID3D12Device>(*queue_desc.device);
	// Create a direct command queue
	ID3D12CommandQueue* commandQueue = nullptr;
	{
		D3D12_COMMAND_QUEUE_DESC qdesc = {};
		qdesc.Type = dx12_queue_type[(int)type];
		qdesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		qdesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		qdesc.NodeMask = 0;
		HRESULT hr = device->CreateCommandQueue(&qdesc, IID_PPV_ARGS(&commandQueue));
		if (FAILED(hr) || !commandQueue) {
			throw std::exception("Failed to create D3D12 command queue");
		}}

	RHI_FENCE_DESC desc;
	desc.device = queue_desc.device;
	desc.flags = fence_flags_none;
	desc.initial_value = 0;
	std::unique_ptr<RHI_OBJECT> fence = dx12_create_fence(desc);
	return std::make_unique<RHI_COMMAND_QUEUE>(new DX_COMMAND_QUEUE_HANDLE(commandQueue), std::move(fence));
}

std::unique_ptr<RHI_OBJECT> dx12_create_graphics_command_queue(const RHI_COMMAND_QUEUE_DESC& queue_desc) {
	return dx12_create_command_queue(queue_desc, queue_type_graphics);
}

std::unique_ptr<RHI_OBJECT> dx12_create_compute_command_queue(const RHI_COMMAND_QUEUE_DESC& queue_desc) {
	return dx12_create_command_queue(queue_desc, queue_type_compute);
}

std::unique_ptr<RHI_OBJECT> dx12_create_copy_command_queue(const RHI_COMMAND_QUEUE_DESC& queue_desc) {
	return dx12_create_command_queue(queue_desc, queue_type_copy);
}

void dx12_command_queue_execute_command_buffers(RHI_COMMAND_BUUFER_LIST& command_buffers) {
	
	auto& command_list = command_buffers.get_list();
	auto queue = dx_rhi_get_interface<ID3D12CommandQueue>(command_buffers.get_queue());
	for (auto& cmd_buffer : command_list) {
		// Execute the command list
		auto cmd_list = dx_rhi_get_interface<ID3D12GraphicsCommandList>(*cmd_buffer);
		queue->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList* const*>(&cmd_list));
	}
}

void dx12_command_queue_wait_command_buffers(RHI_COMMAND_BUUFER_LIST& command_buffers) {

	HANDLE eventHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);	
	if (!eventHandle) {
		throw std::exception("Failed to create event for command queue wait");
	}
	
	auto& queue = command_buffers.get_queue();
	auto iqueue = dx_rhi_get_interface<ID3D12CommandQueue>(queue);
	auto fence = dx_rhi_get_interface<ID3D12Fence>(queue.get_fence());
	iqueue->Signal(fence, command_buffers.get_counter());
	if (fence->GetCompletedValue() < command_buffers.get_counter()) {
		// Wait for the fence to be signaled
		fence->SetEventOnCompletion(command_buffers.get_counter(), eventHandle);
		WaitForSingleObjectEx(eventHandle, INFINITE, FALSE);
	}
	CloseHandle(eventHandle);
	command_buffers.increment_counter();
}
