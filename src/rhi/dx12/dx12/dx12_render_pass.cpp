#include "dx12_render_pass.hpp"
#include "dx12_command_buffer.hpp"
#include "dx12_command_queue.hpp"

std::unique_ptr<RHI_OBJECT> dx12_render_pass_create(RHI_RENDER_PASS_DESC& desc) {

	ID3D12Device* device = desc.device().handle<DX_DEVICE_HANDLE>();
	ID3D12Resource* buffer = desc.surface->handle<DX_RESOURCE_HANDLE>();
	D3D12_RENDER_TARGET_VIEW_DESC rtv_desc = {};
	rtv_desc.Format = dx12_resource_format_type[(int)desc.format];
	rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	ID3D12DescriptorHeap* heap = desc.pool.handle<DX_DESCRIPTOR_POOL>();
	std::unique_ptr<D3D12_CPU_DESCRIPTOR_HANDLE> rtvHandle = dx12_helpers_get_descriptor_heap_handle(device, heap, desc.buffer_index);
	device->CreateRenderTargetView(buffer, &rtv_desc, *rtvHandle);
	
	return std::unique_ptr<RHI_OBJECT>(new RHI_RENDER_PASS(
		new RHI_NATIVE_HANDLE(
			reinterpret_cast<uint8_t*>(rtvHandle.release())
		),
		desc.device,
		std::move(desc.surface),
		desc.synchronized
	));
}

void dx12_render_pass_begin(RHI_OBJECT& render_pass, RHI_OBJECT& command_buffer) {

	dx12_command_buffer_begin_record(command_buffer);

	static float clearColor[] = { 0.1f, 0.2f, 0.4f, 1.0f }; // RGBA

	auto& render_pass_impl = dynamic_cast<RHI_RENDER_PASS&>(render_pass);
	ID3D12Resource* ibuffer = render_pass_impl.get_surface().handle<DX_RESOURCE_HANDLE>();
	ID3D12GraphicsCommandList* command_buffer_impl = command_buffer.handle<DX_COMMAND_BUFFER_HANDLE>();
	
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = ibuffer;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	
	D3D12_CPU_DESCRIPTOR_HANDLE& rtv_handle = *reinterpret_cast<D3D12_CPU_DESCRIPTOR_HANDLE*>(render_pass.handle<RHI_NATIVE_HANDLE>().get_handle());
	D3D12_CPU_DESCRIPTOR_HANDLE* dsv_handle = reinterpret_cast<D3D12_CPU_DESCRIPTOR_HANDLE*>(render_pass_impl.get_depth_buffer().handle<RHI_NATIVE_HANDLE>().get_handle());
	command_buffer_impl->ResourceBarrier(1, &barrier);
	command_buffer_impl->OMSetRenderTargets(1, &rtv_handle, FALSE, dsv_handle);
	command_buffer_impl->ClearRenderTargetView(
		rtv_handle,
		clearColor,
		0,
		nullptr
	);

}

void dx12_render_pass_end(RHI_OBJECT& render_pass, RHI_OBJECT& command_queue, RHI_OBJECT& command_buffer) {

	auto& render_pass_impl = dynamic_cast<RHI_RENDER_PASS&>(render_pass);
	ID3D12Resource* ibuffer = render_pass_impl.get_surface().handle<DX_RESOURCE_HANDLE>();
	ID3D12GraphicsCommandList* command_buffer_impl = command_buffer.handle<DX_COMMAND_BUFFER_HANDLE>();

	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = ibuffer;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	
	command_buffer_impl->ResourceBarrier(1, &barrier);
	
	dx12_command_buffer_end_record(command_buffer);

	dx12_command_queue_execute(command_queue, command_buffer, render_pass_impl.is_synchronized());
}