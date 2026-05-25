#include "dx12_render_pass.hpp"
#include "dx12_command_buffer.hpp"
#include "dx12_command_queue.hpp"

RHI_RENDER_PASS* dx12_render_pass_create(const RHI_RENDER_PASS_DESC* const desc) {

	ASSERT_PTR(desc);
	ASSERT_PTR(desc->device);
	ASSERT_PTR(desc->render_target_view);

	DX_RENDER_PASS* result = new DX_RENDER_PASS();
	ASSERT_PTR(result);
	result->device = desc->device;
	result->render_target_view = desc->render_target_view;

	return result;
}

void dx12_render_pass_execute_rt_mode(const RHI_RENDER_PASS* const render_pass, RHI_COMMAND_BUFFER* const command_buffer,
	fptr_render_pass_on_execute callback) {

	ASSERT_PTR(render_pass);
	ASSERT_PTR(render_pass->device);
	ASSERT_PTR(render_pass->render_target_view);
	ASSERT_PTR(render_pass->pipeline);
	ASSERT_PTR(render_pass->pipeline->layout);
	ASSERT_PTR(command_buffer);

	DX_DEVICE* device_impl = static_cast<DX_DEVICE*>(render_pass->device);
	ASSERT_PTR(device_impl->resources_heap);
	DX_VIEW* render_target_view_impl = static_cast<DX_VIEW*>(render_pass->render_target_view);

	DX_RT_PIPELINE* pipeline_impl = static_cast<DX_RT_PIPELINE*>(render_pass->pipeline);
	ID3D12GraphicsCommandList* i_command_buffer = *static_cast<DX_COMMAND_BUFFER*>(command_buffer);
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList5> i_command_buffer_5;
	ASSERT_SUCCESS(i_command_buffer->QueryInterface(IID_PPV_ARGS(&i_command_buffer_5)));
	ASSERT_PTR(i_command_buffer_5);
	ASSERT_PTR(device_impl->resources_heap.get());
	ID3D12DescriptorHeap* heaps[] =
	{
		*device_impl->resources_heap.get()
	};

	dx12_command_buffer_resource_transition(nullptr, nullptr,
		D3D12_RESOURCE_STATE_RENDER_TARGET, false, []() {});

	dx12_command_buffer_resource_transition(i_command_buffer,
		*static_cast<DX_BUFFER*>(render_target_view_impl->resource.get()),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		true,
		[&]() {

			i_command_buffer_5->SetDescriptorHeaps(_countof(heaps), heaps);
			i_command_buffer_5->SetComputeRootSignature(*static_cast<DX_PIPELINE_LAYOUT*>(render_pass->pipeline->layout));
			i_command_buffer_5->SetComputeRootDescriptorTable(0, device_impl->resources_heap->descriptor_handle.gpu_descriptor_handle);
			i_command_buffer_5->SetPipelineState1(static_cast<DX_RT_PIPELINE&>(*pipeline_impl));
			if (callback)
				callback();
		});
}

void dx12_render_pass_execute_raster_mode(const RHI_RENDER_PASS* const render_pass, 
	RHI_COMMAND_BUFFER* const command_buffer,
	fptr_render_pass_on_execute callback) {
	
	ASSERT_PTR(command_buffer);
	ASSERT_PTR(render_pass);
	ASSERT_PTR(render_pass->device);
	DX_DEVICE* device_impl = static_cast<DX_DEVICE*>(render_pass->device);
	ID3D12Device* i_device = *device_impl;
	ASSERT_PTR(i_device);
	ID3D12GraphicsCommandList* i_command_buffer = *static_cast<DX_COMMAND_BUFFER*>(command_buffer);
	ASSERT_PTR(i_command_buffer);

	DX_RASTER_PIPELINE* pipeline_impl = static_cast<DX_RASTER_PIPELINE*>(render_pass->pipeline);
	DX_VIEW* render_target_view_impl = static_cast<DX_VIEW*>(render_pass->render_target_view);
	DX_VIEW* depth_buffer_view_impl = static_cast<DX_VIEW*>(render_pass->depth_buffer_view);
		
	const RHI_VIEWPORT& vp = render_pass->view_port;
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

	DX_RESOURCE* resource_impl = static_cast<DX_BUFFER*>(render_target_view_impl->resource.get());
	ASSERT_PTR(resource_impl);
	dx12_command_buffer_resource_transition(i_command_buffer,
		resource_impl,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		true,
		[&]() {
		
			D3D12_CPU_DESCRIPTOR_HANDLE* dsv_handle = nullptr;
			if (depth_buffer_view_impl) {

				dsv_handle = &depth_buffer_view_impl->cpu_descriptor_handle;
			}

			static float clearColor[] = { 0.1f, 0.2f, 0.4f, 1.0f };

			i_command_buffer->RSSetViewports(1, &dx_vp);
			i_command_buffer->RSSetScissorRects(1, &dx_scissor);
			i_command_buffer->OMSetRenderTargets(1, &render_target_view_impl->cpu_descriptor_handle, FALSE, dsv_handle);
			i_command_buffer->ClearRenderTargetView(
				render_target_view_impl->cpu_descriptor_handle,
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

			if (pipeline_impl) {

				ASSERT_PTR(device_impl->resources_heap.get());				
				ID3D12DescriptorHeap* resource_heap = *device_impl->resources_heap.get();
				ASSERT_PTR(resource_heap);
				ID3D12DescriptorHeap* sampler_heap = nullptr;
				if (device_impl->sampler_heap.get())
					sampler_heap = *device_impl->sampler_heap.get();
				ID3D12PipelineState* i_pipeline = *pipeline_impl;
				ASSERT_PTR(i_pipeline);
				i_command_buffer->SetPipelineState(i_pipeline);
				ID3D12RootSignature* i_signature = *static_cast<DX_PIPELINE_LAYOUT*>(pipeline_impl->layout);
				ASSERT_PTR(i_signature);
				i_command_buffer->SetGraphicsRootSignature(i_signature);
				i_command_buffer->SetGraphicsRootDescriptorTable(0, device_impl->resources_heap->descriptor_handle.gpu_descriptor_handle);		
				if (sampler_heap) {
					ID3D12DescriptorHeap* heaps[] =
					{
						resource_heap,
						sampler_heap
					};
					i_command_buffer->SetDescriptorHeaps(2, heaps);
					i_command_buffer->SetGraphicsRootDescriptorTable(1, device_impl->sampler_heap->descriptor_handle.gpu_descriptor_handle);
				}
				else {
					i_command_buffer->SetDescriptorHeaps(1, &resource_heap);
				}
			}

			if (callback)
				callback();

	});

	dx12_command_buffer_resource_transition(i_command_buffer,
		resource_impl,
		D3D12_RESOURCE_STATE_PRESENT,
		false,
		[&]() {});
}
