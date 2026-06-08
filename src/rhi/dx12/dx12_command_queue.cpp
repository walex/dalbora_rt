#include "dx12_command_queue.hpp"
#include "dx12_fence.hpp"

RHI_COMMAND_QUEUE* dx12_create_command_queue(const RHI_COMMAND_QUEUE_DESC* const desc, const queue_type type) {
	
	ASSERT_PTR(desc);
	ASSERT_PTR(desc->device);

	ID3D12Device* i_device = *static_cast<const DX_DEVICE*>(desc->device);

	// Create a direct command queue
	ID3D12CommandQueue* i_cmd_queue = nullptr;
	D3D12_COMMAND_QUEUE_DESC qdesc = {};
	qdesc.Type = dx12_queue_type[(int)type];
	qdesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	qdesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	qdesc.NodeMask = 0;
	ASSERT_SUCCESS(i_device->CreateCommandQueue(&qdesc, IID_PPV_ARGS(&i_cmd_queue)));
	ASSERT_PTR(i_cmd_queue);

	DX_COMMAND_QUEUE* result = new DX_COMMAND_QUEUE();
	ASSERT_PTR(result);
	result->set_handle(i_cmd_queue);

	RHI_FENCE_DESC fence_desc;
	fence_desc.device = desc->device;
	fence_desc.flags = fence_flags_none;
	fence_desc.initial_value = 0;

	result->fence.reset(dx12_fence_create(&fence_desc));
	ASSERT_PTR(result->fence);

	return result;
}

RHI_COMMAND_QUEUE* dx12_command_queue_create_for_render(const RHI_COMMAND_QUEUE_DESC* desc) {
	return dx12_create_command_queue(desc, queue_type_graphics);
}

RHI_COMMAND_QUEUE* dx12_command_queue_create_for_compute(const RHI_COMMAND_QUEUE_DESC* desc) {
	return dx12_create_command_queue(desc, queue_type_compute);
}

RHI_COMMAND_QUEUE* dx12_command_queue_create_for_copy(const RHI_COMMAND_QUEUE_DESC* desc) {
	return dx12_create_command_queue(desc, queue_type_copy);
}

void dx12_command_queue_execute(RHI_COMMAND_QUEUE* const command_queue, const bool wait_completion,
	fptr_command_queue_on_execute callback) {

	ASSERT_PTR(command_queue);

	std::vector<RHI_COMMAND_BUFFER*> command_buffer_list;
	ID3D12CommandQueue* i_cmd_queue = *static_cast<DX_COMMAND_QUEUE*>(command_queue);
	ASSERT_PTR(i_cmd_queue);
	callback(static_cast<RHI_VOID_PTR>(i_cmd_queue), &command_buffer_list);
	size_t list_size = command_buffer_list.size();
	if (list_size > 0) {
		std::vector<ID3D12CommandList*> native_list(list_size);
		for (int i = 0; i < list_size; i++)
			native_list[i] = *static_cast<DX_COMMAND_BUFFER*>(command_buffer_list[i]);
		i_cmd_queue->ExecuteCommandLists(1, native_list.data());
		if (wait_completion == true) {
			dx12_command_queue_sync(command_queue);
		}
	}
}

void dx12_command_queue_sync(RHI_COMMAND_QUEUE* command_queue) {

	ASSERT_PTR(command_queue);

	const DX_COMMAND_QUEUE* command_queue_impl = static_cast<const DX_COMMAND_QUEUE*>(command_queue);
	ID3D12CommandQueue* i_cmd_queue = *command_queue_impl;
	ASSERT_PTR(i_cmd_queue);
	ID3D12Fence* i_fence = *static_cast<const DX_FENCE*>(command_queue->fence.get());
	ASSERT_PTR(i_fence);
	HANDLE eventHandle = command_queue_impl->event_handle;
	const uint64_t fc = ++command_queue->fence_counter;
	ASSERT_EXPR(fc != UINT64_MAX);
	i_cmd_queue->Signal(i_fence, fc);
	if (i_fence->GetCompletedValue() < fc) {
		// Wait for the fence to be signaled
		i_fence->SetEventOnCompletion(fc, eventHandle);
;		WaitForSingleObjectEx(eventHandle, INFINITE, FALSE);
	}
}
