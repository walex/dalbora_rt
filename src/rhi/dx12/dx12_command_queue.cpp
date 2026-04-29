#include "dx12_command_queue.hpp"
#include "dx12_fence.hpp"

std::unique_ptr<RHI_OBJECT> dx12_create_command_queue(const RHI_COMMAND_QUEUE_DESC& desc, queue_type type) {
	
	ID3D12Device* device = desc.device().handle<DX_DEVICE_HANDLE>();
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

	RHI_FENCE_DESC fence_desc(desc.device);
	fence_desc.device = desc.device;
	fence_desc.flags = fence_flags_none;
	fence_desc.initial_value = 0;
	std::unique_ptr<RHI_OBJECT> fence = dx12_fence_create(fence_desc);
	return std::make_unique<RHI_COMMAND_QUEUE>(new DX_COMMAND_QUEUE_HANDLE(commandQueue), std::move(fence));
}

std::unique_ptr<RHI_OBJECT> dx12_command_queue_create_for_render(const RHI_COMMAND_QUEUE_DESC& queue_desc) {
	return dx12_create_command_queue(queue_desc, queue_type_graphics);
}

std::unique_ptr<RHI_OBJECT> dx12_command_queue_create_for_compute(const RHI_COMMAND_QUEUE_DESC& queue_desc) {
	return dx12_create_command_queue(queue_desc, queue_type_compute);
}

std::unique_ptr<RHI_OBJECT> dx12_command_queue_create_for_copy(const RHI_COMMAND_QUEUE_DESC& queue_desc) {
	return dx12_create_command_queue(queue_desc, queue_type_copy);
}

void dx12_command_queue_execute_list(RHI_COMMAND_BUUFER_LIST& command_buffer_list, bool sync) {
	
	auto& list = command_buffer_list.get_list();
	std::vector<ID3D12CommandList*> native_list(list.size());
	for (int i = 0; i < list.size(); i++) {
		ID3D12GraphicsCommandList* cmd_buffer = list[i]->handle<DX_COMMAND_BUFFER_HANDLE>();
		cmd_buffer->Close();
		native_list[i] = cmd_buffer;
	}
	ID3D12CommandQueue* queue = command_buffer_list.get_queue().handle<DX_COMMAND_QUEUE_HANDLE>();
	queue->ExecuteCommandLists(1, native_list.data());
	if (sync)
		dx12_command_queue_wait(command_buffer_list);
}

void dx12_command_queue_wait(RHI_COMMAND_BUUFER_LIST& command_buffers) {

	HANDLE eventHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);	
	if (!eventHandle) {
		throw std::exception("Failed to create event for command queue wait");
	}
	auto& queue = reinterpret_cast<RHI_COMMAND_QUEUE&>(command_buffers.get_queue());
	ID3D12CommandQueue* iqueue = queue.handle<DX_COMMAND_QUEUE_HANDLE>();
	ID3D12Fence* fence = queue.get_fence().handle<DX_FENCE_HANDLE>();
	iqueue->Signal(fence, command_buffers.get_counter());
	if (fence->GetCompletedValue() < command_buffers.get_counter()) {
		// Wait for the fence to be signaled
		fence->SetEventOnCompletion(command_buffers.get_counter(), eventHandle);
		WaitForSingleObjectEx(eventHandle, INFINITE, FALSE);
	}
	CloseHandle(eventHandle);
	command_buffers.increment_counter();
}

void dx12_command_queue_execute(RHI_OBJECT& queue, RHI_OBJECT& cmd_buffer, bool sync) {

	std::vector<RHI_OBJECT*> cmd_buffers(1);
	cmd_buffers.push_back(&cmd_buffer);
	RHI_COMMAND_BUUFER_LIST command_buffer_list(queue, std::move(cmd_buffers));
	dx12_command_queue_execute_list(command_buffer_list, sync);		
}

void dx12_command_queue_execute_synchronized(RHI_OBJECT& queue, RHI_OBJECT& cmd_buffer) {
	dx12_command_queue_execute(queue, cmd_buffer, true);
}

void dx12_command_queue_execute_list_synchronized(RHI_COMMAND_BUUFER_LIST& command_buffers) {
	dx12_command_queue_execute_list(command_buffers, true);
}