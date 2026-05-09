#include "dx12_render_pass.hpp"
#include "dx12_command_buffer.hpp"
#include "dx12_command_queue.hpp"

std::unique_ptr<RHI_RENDER_PASS> dx12_render_pass_create(const RHI_RENDER_PASS_DESC& desc) {

	DX_DEVICE& device_impl = reinterpret_cast<DX_DEVICE&>(desc.device.get());
	DX_HEAP* heap_impl = device_impl.get_rtv_heap();
	if (heap_impl == nullptr) {
		throw std::exception("NO heap found for rtv.");
	}
	ID3D12Device* i_device = static_cast<ID3D12Device*>(device_impl);
	ID3D12Resource* i_buffer = static_cast<ID3D12Resource*>(*desc.render_target.get());
	D3D12_RENDER_TARGET_VIEW_DESC rtv_desc = {};
	rtv_desc.Format = dx12_resource_format_type[(int)desc.format];
	rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	ID3D12DescriptorHeap* i_heap = static_cast<ID3D12DescriptorHeap*>(*heap_impl);
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
	auto* pipeline_impl = static_cast<RHI_GRAPHICS_PIPELINE*>(render_pass);
	auto& render_target_impl = static_cast<RHI_TEXTURE_2D&>(render_pass);
	ID3D12GraphicsCommandList* command_buffer_impl = reinterpret_cast<ID3D12GraphicsCommandList*>(static_cast<ID3D12CommandList*>(command_buffer));

	RHI_VIEWPORT& vp = static_cast<RHI_VIEWPORT&>(render_pass);
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
	
	if (pipeline_impl) {
		
		if (pipeline_impl->get_type() == pipeline_type_raster) {
			auto& dx_pipeline_impl = reinterpret_cast<DX_RASTER_PIPELINE&>(*pipeline_impl);
			DX_PIPELINE_LAYOUT& layout = reinterpret_cast<DX_PIPELINE_LAYOUT&>(static_cast<RHI_PIPELINE_LAYOUT&>(dx_pipeline_impl));
			ID3D12PipelineState* i_state = static_cast<ID3D12PipelineState*>(dx_pipeline_impl);
			ID3D12RootSignature* i_sig = static_cast<ID3D12RootSignature*>(layout);
			command_buffer_impl->SetPipelineState(i_state);
			command_buffer_impl->SetGraphicsRootSignature(i_sig);
		}
		else if (pipeline_impl->get_type() == pipeline_type_rt) {
			throw std::exception("pipeline type rt not implemented.");
		}
		else {
			throw std::exception("pipeline type not implemented.");
		}		
	}

	DX_DEVICE& device_impl = static_cast<DX_DEVICE&>(static_cast<RHI_DEVICE&>(render_pass));
	ID3D12DescriptorHeap* resource_heap = static_cast<ID3D12DescriptorHeap*>(*device_impl.get_resources_heap());

	D3D12_GPU_DESCRIPTOR_HANDLE resource_heap_handle =
		resource_heap->GetGPUDescriptorHandleForHeapStart();
	command_buffer_impl->SetDescriptorHeaps(1, &resource_heap);
	command_buffer_impl->SetGraphicsRootDescriptorTable(0, resource_heap_handle);
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
