#include "dx12_render_pass.hpp"
#include "dx12_command_buffer.hpp"
#include "dx12_command_queue.hpp"

std::unique_ptr<RHI_RENDER_PASS> dx12_render_pass_create(const RHI_RENDER_PASS_DESC& desc) {

	return std::make_unique<DX_RENDER_PASS>(
		desc.device,
		desc.render_target
	);
}

void dx12_render_pass_execute(RHI_RENDER_PASS& render_pass, RHI_COMMAND_BUFFER& command_buffer,
	fptr_render_pass_on_execute callback) {
	
	static float clearColor[] = { 0.1f, 0.2f, 0.4f, 1.0f }; // RGBA

	RHI_DEPTH_BUFFER* depth_buffer_impl = render_pass;
	RHI_GRAPHICS_PIPELINE* pipeline_impl = render_pass;
	RHI_TEXTURE_2D& render_target_impl = render_pass;
	ID3D12GraphicsCommandList* command_buffer_impl = command_buffer;

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

	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = render_target_impl;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	D3D12_CPU_DESCRIPTOR_HANDLE& rtv_handle = static_cast<DX_TEXTURE_2D&>(static_cast<RHI_TEXTURE_2D&>(render_pass));
	D3D12_CPU_DESCRIPTOR_HANDLE* dsv_handle = nullptr;
	if (depth_buffer_impl) {

		auto rd = static_cast<DX_DEPTH_BUFFER*>(depth_buffer_impl);
		dsv_handle = &static_cast<D3D12_CPU_DESCRIPTOR_HANDLE&>(*rd);
	}
	
	if (pipeline_impl) {
		
		if (pipeline_impl->get_type() == pipeline_type_raster) {
			DX_RASTER_PIPELINE& dx_pipeline_impl = reinterpret_cast<DX_RASTER_PIPELINE&>(*pipeline_impl);
			DX_DEVICE& device_impl = static_cast<DX_DEVICE&>(static_cast<RHI_DEVICE&>(render_pass));
			ID3D12DescriptorHeap* resource_heap = *device_impl.get_resources_heap();
			ID3D12DescriptorHeap* sampler_heap = *device_impl.get_sampler_heap();
			command_buffer_impl->SetPipelineState(dx_pipeline_impl);
			command_buffer_impl->SetGraphicsRootSignature(static_cast<DX_PIPELINE_LAYOUT&>(dx_pipeline_impl));
			ID3D12DescriptorHeap* heaps[] =
			{
				resource_heap,
				sampler_heap
			};
			command_buffer_impl->SetDescriptorHeaps(2, heaps);

			ID3D12Device* i_device = static_cast<DX_DEVICE&>(static_cast<RHI_DEVICE&>(render_pass));
			auto resource_heap_handle = dx12_helpers_get_read_only_descriptor_heap_handle(i_device, resource_heap, 0);
			command_buffer_impl->SetGraphicsRootDescriptorTable(0, *resource_heap_handle);
			
			auto sampler_heap_handle = dx12_helpers_get_read_only_descriptor_heap_handle(i_device, sampler_heap, 0);
			command_buffer_impl->SetGraphicsRootDescriptorTable(1, *sampler_heap_handle);
		}
		else if (pipeline_impl->get_type() == pipeline_type_rt) {
			throw std::exception("pipeline type rt not implemented.");
		}
		else {
			throw std::exception("pipeline type not implemented.");
		}		
	}

	command_buffer_impl->RSSetViewports(1, &dx_vp);
	command_buffer_impl->RSSetScissorRects(1, &dx_scissor);
	command_buffer_impl->ResourceBarrier(1, &barrier);
	command_buffer_impl->OMSetRenderTargets(1, &rtv_handle, FALSE, dsv_handle);
	
	command_buffer_impl->ClearRenderTargetView(
		rtv_handle,
		clearColor,
		0,
		nullptr
	);
	
	if (dsv_handle) {
			command_buffer_impl->ClearDepthStencilView(
				*dsv_handle,                         // D3D12_CPU_DESCRIPTOR_HANDLE
				D3D12_CLEAR_FLAG_DEPTH,            // qué limpiar
				1.0f,                              // depth clear value
				0,                                 // stencil clear value
				0,                                 // num rects
				nullptr                            // rects
			);
	}

	if (callback)
		callback();

	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	command_buffer_impl->ResourceBarrier(1, &barrier);
}
