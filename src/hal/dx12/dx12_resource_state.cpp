#include "dx12_resource_state.hpp"

void dx12_resource_state_transition(HAL_OBJECT& command_buffer, HAL_RESOURCE& resource, resource_state old_state, resource_state new_state) {

	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = dx_hal_get_interface<ID3D12Resource>(resource);
	barrier.Transition.StateBefore = dx12_resource_state_type[old_state];
	barrier.Transition.StateAfter = dx12_resource_state_type[new_state];
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	auto commandList = dx_hal_get_interface<ID3D12GraphicsCommandList>(command_buffer);
	commandList->ResourceBarrier(1, &barrier);
}
