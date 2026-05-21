#include "dx12_render_pass.hpp"
#include "dx12_command_buffer.hpp"
#include "dx12_command_queue.hpp"

std::unique_ptr<RHI_RENDER_PASS> dx12_render_pass_create(const RHI_RENDER_PASS_DESC& desc) {

	return std::make_unique<DX_RENDER_PASS>(
		desc.device,
		desc.render_target
	);
}

void dx12_render_pass_execute_rt_mode(RHI_RENDER_PASS& render_pass, RHI_COMMAND_BUFFER& command_buffer,
	fptr_render_pass_on_execute callback) {

	RHI_DEPTH_BUFFER* depth_buffer_impl = render_pass;
	RHI_GRAPHICS_PIPELINE* pipeline_impl = render_pass;
	RHI_TEXTURE_2D& render_target_impl = render_pass;
	DX_DEVICE& device_impl = static_cast<DX_DEVICE&>(static_cast<RHI_DEVICE&>(render_pass));
	ID3D12GraphicsCommandList* i_command_buffer = command_buffer;
	DX_RT_PIPELINE& dx_pipeline_impl = static_cast<DX_RT_PIPELINE&>(*pipeline_impl);
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList5> i_command_buffer_5;

	resource_state old_state = render_target_impl.get_current_state();
	D3D12_RESOURCE_BARRIER barrier = {};
	if (old_state != resource_state_rt_render_target) {
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = render_target_impl;
		barrier.Transition.StateBefore = dx12_resource_state_type[old_state];
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		i_command_buffer->ResourceBarrier(1, &barrier);
		render_target_impl.set_current_state(resource_state_rt_render_target);
	}
	i_command_buffer->QueryInterface(IID_PPV_ARGS(&i_command_buffer_5));
	ID3D12DescriptorHeap* heaps[] =
	{
		*device_impl.get_resources_heap()
	};

	auto handle = dx12_helpers_get_read_only_descriptor_heap_handle(device_impl, *device_impl.get_resources_heap(), 0);
	i_command_buffer_5->SetDescriptorHeaps(_countof(heaps), heaps);
	i_command_buffer_5->SetComputeRootSignature(static_cast<DX_PIPELINE_LAYOUT&>(static_cast<DX_RT_PIPELINE&>(*pipeline_impl)));
	i_command_buffer_5->SetComputeRootDescriptorTable(0, *handle.get());
	i_command_buffer_5->SetPipelineState1(static_cast<DX_RT_PIPELINE&>(*pipeline_impl));

	if (callback)
		callback();

	if (old_state != resource_state_rt_render_target) {
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		barrier.Transition.StateAfter = dx12_resource_state_type[old_state];
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		i_command_buffer->ResourceBarrier(1, &barrier);
		render_target_impl.set_current_state(old_state);
	}
}

void dx12_render_pass_execute_raster_mode(RHI_RENDER_PASS& render_pass, RHI_COMMAND_BUFFER& command_buffer,
	fptr_render_pass_on_execute callback) {
	
	static float clearColor[] = { 0.1f, 0.2f, 0.4f, 1.0f }; // RGBA

	RHI_DEPTH_BUFFER* depth_buffer_impl = render_pass;
	RHI_GRAPHICS_PIPELINE* pipeline_impl = render_pass;
	RHI_TEXTURE_2D& render_target_impl = render_pass;
	DX_DEVICE& device_impl = static_cast<DX_DEVICE&>(static_cast<RHI_DEVICE&>(render_pass));
	ID3D12GraphicsCommandList* i_command_buffer = command_buffer;

	RHI_VIEWPORT& vp = render_pass;
	D3D12_VIEWPORT dx_vp;
	dx_vp.TopLeftX = vp.x;
	dx_vp.TopLeftY = vp.y;
	dx_vp.Width = vp.width;
	dx_vp.Height = vp.height;
	dx_vp.MinDepth = vp.min_z;
	dx_vp.MaxDepth = vp.max_z;
	D3D12_RECT dx_scissor = {};
	dx_scissor.left = 0;
	dx_scissor.top = 0;
	dx_scissor.right = (LONG)vp.width;
	dx_scissor.bottom = (LONG)vp.height;

	resource_state old_state = render_target_impl.get_current_state();
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.pResource = render_target_impl;
	if (old_state != resource_state_raster_render_target) {
		barrier.Transition.StateBefore = dx12_resource_state_type[old_state];
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		i_command_buffer->ResourceBarrier(1, &barrier);
		render_target_impl.set_current_state(resource_state_raster_render_target);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE& rtv_handle = static_cast<DX_TEXTURE_2D&>(static_cast<RHI_TEXTURE_2D&>(render_pass));
	D3D12_CPU_DESCRIPTOR_HANDLE* dsv_handle = nullptr;
	if (depth_buffer_impl) {

		auto rd = static_cast<DX_DEPTH_BUFFER*>(depth_buffer_impl);
		dsv_handle = &static_cast<D3D12_CPU_DESCRIPTOR_HANDLE&>(*rd);
	}
	
	i_command_buffer->RSSetViewports(1, &dx_vp);
	i_command_buffer->RSSetScissorRects(1, &dx_scissor);

	i_command_buffer->OMSetRenderTargets(1, &rtv_handle, FALSE, dsv_handle);

	i_command_buffer->ClearRenderTargetView(
		rtv_handle,
		clearColor,
		0,
		nullptr
	);

	if (dsv_handle) {
		i_command_buffer->ClearDepthStencilView(
			*dsv_handle,                         // D3D12_CPU_DESCRIPTOR_HANDLE
			D3D12_CLEAR_FLAG_DEPTH,            // qué limpiar
			1.0f,                              // depth clear value
			0,                                 // stencil clear value
			0,                                 // num rects
			nullptr                            // rects
		);
	}

	// ASSERT HERE
	if (pipeline_impl) {
		
		DX_RASTER_PIPELINE& dx_pipeline_impl = reinterpret_cast<DX_RASTER_PIPELINE&>(*pipeline_impl);
		ID3D12DescriptorHeap* resource_heap = *device_impl.get_resources_heap();
		auto sampler_heap_impl = device_impl.get_sampler_heap();
		ID3D12DescriptorHeap* sampler_heap = nullptr;
		if (sampler_heap_impl)
			sampler_heap = *sampler_heap_impl;
		i_command_buffer->SetPipelineState(dx_pipeline_impl);
		i_command_buffer->SetGraphicsRootSignature(static_cast<DX_PIPELINE_LAYOUT&>(dx_pipeline_impl));
		ID3D12DescriptorHeap* heaps[] =
		{
			resource_heap,
			sampler_heap
		};
		i_command_buffer->SetDescriptorHeaps(sampler_heap_impl?2:1, heaps);

		ID3D12Device* i_device = static_cast<DX_DEVICE&>(static_cast<RHI_DEVICE&>(render_pass));
		auto resource_heap_handle = dx12_helpers_get_read_only_descriptor_heap_handle(i_device, resource_heap, 0);
		i_command_buffer->SetGraphicsRootDescriptorTable(0, *resource_heap_handle);

		if (sampler_heap) {
			auto sampler_heap_handle = dx12_helpers_get_read_only_descriptor_heap_handle(i_device, sampler_heap, 0);
			i_command_buffer->SetGraphicsRootDescriptorTable(1, *sampler_heap_handle);
		}
	}	

	if (callback)
		callback();
	
	// leave render target in present mode
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	i_command_buffer->ResourceBarrier(1, &barrier);
	render_target_impl.set_current_state(resource_state_present);
}
