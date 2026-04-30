#include "dx12_render_pass.hpp"
#include "dx12_command_queue.hpp"

std::unique_ptr<RHI_OBJECT> dx12_render_pass_create(RHI_RENDER_PASS_DESC& desc) {

	ID3D12Device* device = desc.device().handle<DX_DEVICE_HANDLE>();
	ID3D12Resource* buffer = desc.surface->handle<DX_RESOURCE_HANDLE>();
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = dx12_resource_format_type[(int)desc.format];
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	std::unique_ptr<D3D12_CPU_DESCRIPTOR_HANDLE> rtvHandle = dx12_helpers_get_descriptor_heap_handle(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, desc.buffer_index);
	device->CreateRenderTargetView(buffer, &rtvDesc, *rtvHandle);
	
	return std::unique_ptr<RHI_OBJECT>(new RHI_RENDER_PASS(
		new RHI_NATIVE_HANDLE(
			reinterpret_cast<void*>(rtvHandle.release())
		),
		desc.device.get(),
		std::move(desc.surface)
	));
}

void dx12_render_pass_begin(RHI_OBJECT& render_pass, RHI_OBJECT& command_buffer) {

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
	D3D12_CPU_DESCRIPTOR_HANDLE& rtvHandle = *reinterpret_cast<D3D12_CPU_DESCRIPTOR_HANDLE*>(render_pass.handle<RHI_NATIVE_HANDLE>().get_handle());
	
	command_buffer_impl->ResourceBarrier(1, &barrier);
	command_buffer_impl->ClearRenderTargetView(
		rtvHandle,
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
	command_buffer_impl->Close();

	dx12_command_queue_execute(command_queue, command_buffer);
}