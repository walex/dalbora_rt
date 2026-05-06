#include "dx12_resource_state.hpp"
#include "dx12_command_buffer.hpp"
#include "dx12_command_queue.hpp"

void dx12_resource_state_transition(RHI_OBJECT& command_queue, RHI_OBJECT& command_buffer, RHI_OBJECT& resource, resource_state new_state) {

	auto& resource_impl = reinterpret_cast<RHI_RESOURCE&>(resource);
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = resource_impl.handle<DX_RESOURCE_HANDLE>();
	barrier.Transition.StateBefore = dx12_resource_state_type[resource_impl.get_current_state()];
	barrier.Transition.StateAfter = dx12_resource_state_type[new_state];
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	ID3D12GraphicsCommandList* commandList = command_buffer.handle<DX_COMMAND_BUFFER_HANDLE>();
	dx12_command_buffer_begin_record(command_buffer);
	commandList->ResourceBarrier(1, &barrier);
	dx12_command_buffer_end_record(command_buffer);
	dx12_command_queue_execute_synchronized(command_queue, command_buffer);
	resource_impl.set_current_state(new_state);
}
