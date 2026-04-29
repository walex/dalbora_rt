#include "dx12_resource_state.hpp"

void dx12_resource_state_transition(RHI_OBJECT& command_buffer, RHI_RESOURCE& resource, resource_state new_state) {

	auto iresource = com_query_interface<ID3D12Resource>(resource);
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = iresource.get();
	barrier.Transition.StateBefore = dx12_resource_state_type[resource.get_current_state()];
	barrier.Transition.StateAfter = dx12_resource_state_type[new_state];
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	ID3D12GraphicsCommandList* commandList = command_buffer.handle<DX_COMMAND_BUFFER_HANDLE>();
	commandList->ResourceBarrier(1, &barrier);
}
