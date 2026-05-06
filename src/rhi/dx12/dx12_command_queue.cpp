#include "dx12_command_queue.hpp"
#include "dx12_fence.hpp"

std::unique_ptr<RHI_COMMAND_QUEUE> dx12_create_command_queue(const RHI_COMMAND_QUEUE_DESC& desc, queue_type type) {
	
	ID3D12Device* i_device = static_cast<ID3D12Device*>(desc.device.get());
	// Create a direct command queue
	ID3D12CommandQueue* i_cmd_queue = nullptr;
	{
		D3D12_COMMAND_QUEUE_DESC qdesc = {};
		qdesc.Type = dx12_queue_type[(int)type];
		qdesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		qdesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		qdesc.NodeMask = 0;
		HRESULT hr = i_device->CreateCommandQueue(&qdesc, IID_PPV_ARGS(&i_cmd_queue));
		if (FAILED(hr) || !i_cmd_queue) {
			throw std::exception("Failed to create D3D12 command queue");
		}}

	RHI_FENCE_DESC fence_desc(desc.device);
	fence_desc.device = desc.device;
	fence_desc.flags = fence_flags_none;
	fence_desc.initial_value = 0;
	std::unique_ptr<RHI_FENCE> fence = dx12_fence_create(fence_desc);
	return std::make_unique<DX_COMMAND_QUEUE>(i_cmd_queue, std::move(fence));
}

std::unique_ptr<RHI_COMMAND_QUEUE> dx12_command_queue_create_for_render(const RHI_COMMAND_QUEUE_DESC& queue_desc) {
	return dx12_create_command_queue(queue_desc, queue_type_graphics);
}

std::unique_ptr<RHI_COMMAND_QUEUE> dx12_command_queue_create_for_compute(const RHI_COMMAND_QUEUE_DESC& queue_desc) {
	return dx12_create_command_queue(queue_desc, queue_type_compute);
}

std::unique_ptr<RHI_COMMAND_QUEUE> dx12_command_queue_create_for_copy(const RHI_COMMAND_QUEUE_DESC& queue_desc) {
	return dx12_create_command_queue(queue_desc, queue_type_copy);
}

void dx12_command_queue_wait(RHI_COMMAND_QUEUE& command_queue) {

	HANDLE eventHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);	
	if (!eventHandle) {
		throw std::exception("Failed to create event for command queue wait");
	}
	auto& command_queue_impl = static_cast<DX_COMMAND_QUEUE&>(command_queue);
	ID3D12CommandQueue* i_cmd_queue = static_cast<ID3D12CommandQueue*>(command_queue_impl);
	ID3D12Fence* i_fence = static_cast<ID3D12Fence*>(command_queue_impl);
	auto fc = command_queue.increment_fence_counter();
	i_cmd_queue->Signal(i_fence, fc);
	if (i_fence->GetCompletedValue() < fc) {
		// Wait for the fence to be signaled
		i_fence->SetEventOnCompletion(fc, eventHandle);
		WaitForSingleObjectEx(eventHandle, INFINITE, FALSE);
	}
	CloseHandle(eventHandle);
	
}

void dx12_command_queue_exec(RHI_COMMAND_QUEUE& command_queue, bool wait_completion, fptr_command_queue_on_execute callback) {

	std::vector<RHI_COMMAND_BUFFER*> command_buffer_list;
	ID3D12CommandQueue* i_cmd_queue = static_cast<ID3D12CommandQueue*>(command_queue);
	callback(static_cast<RHI_VOID_PTR>(i_cmd_queue), command_buffer_list);
	size_t list_size = command_buffer_list.size();
	if (list_size > 0) {
		std::vector<ID3D12CommandList*> native_list(list_size);
		for (int i = 0; i < list_size; i++) {
			ID3D12GraphicsCommandList* i_cmd_list = reinterpret_cast<ID3D12GraphicsCommandList*>(static_cast<ID3D12CommandList*>(*command_buffer_list[i]));
			native_list[i] = i_cmd_list;
		}
		i_cmd_queue->ExecuteCommandLists(1, native_list.data());
		if (wait_completion == true) {
			dx12_command_queue_wait(command_queue);
		}
	}
}