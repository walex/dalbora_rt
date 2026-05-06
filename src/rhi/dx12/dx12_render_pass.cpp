#include "dx12_render_pass.hpp"
#include "dx12_command_buffer.hpp"
#include "dx12_command_queue.hpp"

std::unique_ptr<RHI_RENDER_PASS> dx12_render_pass_create(const RHI_RENDER_PASS_DESC& desc) {

	ID3D12Device* i_device = static_cast<ID3D12Device*>(desc.device.get());
	ID3D12Resource* i_buffer = static_cast<ID3D12Resource*>(*desc.render_target.get());
	D3D12_RENDER_TARGET_VIEW_DESC rtv_desc = {};
	rtv_desc.Format = dx12_resource_format_type[(int)desc.format];
	rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	ID3D12DescriptorHeap* i_heap = static_cast<ID3D12DescriptorHeap*>(desc.pool.get());
	std::unique_ptr<D3D12_CPU_DESCRIPTOR_HANDLE> rtvHandle = dx12_helpers_get_descriptor_heap_handle(i_device, i_heap, desc.buffer_index);
	i_device->CreateRenderTargetView(i_buffer, &rtv_desc, *rtvHandle);
	return std::make_unique<DX_RENDER_PASS>(
		desc.device,
		desc.render_target,
		*rtvHandle
	);
}

void dx12_render_pass_execute(RHI_RENDER_PASS& render_pass, RHI_COMMAND_BUFFER& command_buffer,
	fptr_render_pass_on_execute callback) {
	
	static float clearColor[] = { 0.1f, 0.2f, 0.4f, 1.0f }; // RGBA

	auto* depth_buffer_impl = static_cast<RHI_DEPTH_BUFFER*>(render_pass);
	auto& render_target_impl = static_cast<RHI_TEXTURE_2D&>(render_pass);
	ID3D12GraphicsCommandList* command_buffer_impl = reinterpret_cast<ID3D12GraphicsCommandList*>(static_cast<ID3D12CommandList*>(command_buffer));

	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = static_cast<ID3D12Resource*>(render_target_impl);
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	D3D12_CPU_DESCRIPTOR_HANDLE& rtv_handle = static_cast<D3D12_CPU_DESCRIPTOR_HANDLE&>(
												static_cast<DX_RENDER_PASS&>(render_pass));
	D3D12_CPU_DESCRIPTOR_HANDLE* dsv_handle = nullptr;
	if (depth_buffer_impl) {

		auto rd = static_cast<DX_DEPTH_BUFFER*>(depth_buffer_impl);
		dsv_handle = &static_cast<D3D12_CPU_DESCRIPTOR_HANDLE&>(*rd);
	}
	command_buffer_impl->ResourceBarrier(1, &barrier);
	command_buffer_impl->OMSetRenderTargets(1, &rtv_handle, FALSE, dsv_handle);
	command_buffer_impl->ClearRenderTargetView(
		rtv_handle,
		clearColor,
		0,
		nullptr
	);
	
	if (callback)
		callback();

	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	command_buffer_impl->ResourceBarrier(1, &barrier);
}

void dx12_render_pass_set_depth_buffer(RHI_RENDER_PASS& render_pass, RHI_DEPTH_BUFFER* depth_buffer) {

	render_pass.set_depth_buffer(depth_buffer);

}