#include "dx12_render_pass.hpp"
#include "dx12_command_buffer.hpp"
#include "dx12_command_queue.hpp"

RHI_RENDER_PASS* dx12_render_pass_create(const RHI_RENDER_PASS_DESC* const desc) {

	ASSERT_PTR(desc);
	ASSERT_PTR(desc->device);

	DX_RENDER_PASS* result = new DX_RENDER_PASS();
	ASSERT_PTR(result);
	result->device = desc->device;

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
	ASSERT_PTR(command_buffer->buffer_memory_descriptor);

	DX_DEVICE* device_impl = static_cast<DX_DEVICE*>(render_pass->device);

	DX_RT_PIPELINE* pipeline_impl = static_cast<DX_RT_PIPELINE*>(render_pass->pipeline);
	ID3D12GraphicsCommandList* i_command_buffer = *static_cast<DX_COMMAND_BUFFER*>(command_buffer);
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList5> i_command_buffer_5;
	ASSERT_SUCCESS(i_command_buffer->QueryInterface(IID_PPV_ARGS(&i_command_buffer_5)));
	ASSERT_PTR(i_command_buffer_5);

	UINT heap_count = 1;
	// configure heap

	ID3D12DescriptorHeap* resource_heap = *static_cast<DX_MEMORY_DESCRIPTOR*>(command_buffer->buffer_memory_descriptor);
	ASSERT_PTR(resource_heap);

	ID3D12DescriptorHeap* sampler_heap = nullptr;
	if (command_buffer->sampler_memory_descriptor) {
		sampler_heap = *static_cast<DX_MEMORY_DESCRIPTOR*>(command_buffer->sampler_memory_descriptor);
		heap_count++;
	}
	ID3D12DescriptorHeap* heaps[] =
	{
		resource_heap,
		sampler_heap
	};

	D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle 
		= {static_cast<DX_MEMORY_DESCRIPTOR*>(command_buffer->buffer_memory_descriptor)->gpu_handle};
	i_command_buffer_5->SetDescriptorHeaps(heap_count, heaps);
	i_command_buffer_5->SetComputeRootSignature(*static_cast<DX_PIPELINE_LAYOUT*>(render_pass->pipeline->layout));
	i_command_buffer_5->SetComputeRootDescriptorTable(0, gpu_handle);
	i_command_buffer_5->SetPipelineState1(*static_cast<DX_RT_PIPELINE*>(pipeline_impl));
	if (callback)
		callback();
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

	DX_RESOURCE* resource_impl = static_cast<DX_BUFFER*>(render_target_view_impl->buffer);
	ASSERT_PTR(resource_impl);
	
	D3D12_CPU_DESCRIPTOR_HANDLE dsv_handle;
	D3D12_CPU_DESCRIPTOR_HANDLE* dsv_handle_ptr = nullptr;
	if (depth_buffer_view_impl) {

		const DX_MEMORY_DESCRIPTOR_SLOT* dsv_slot = static_cast<const DX_MEMORY_DESCRIPTOR_SLOT*>(depth_buffer_view_impl->memory_descriptor);
		dsv_handle = {dsv_slot->cpu_handle};
		dsv_handle_ptr = &dsv_handle;
	}

	dx12_command_buffer_resource_barrier_transition(i_command_buffer,
		{ resource_impl },
		{ D3D12_RESOURCE_STATE_RENDER_TARGET },
		[&]() {
		
			// configure heap
			ASSERT_PTR(command_buffer->buffer_memory_descriptor);
			ID3D12DescriptorHeap* resource_heap = *static_cast<DX_MEMORY_DESCRIPTOR*>(command_buffer->buffer_memory_descriptor);
			ASSERT_PTR(resource_heap);

			UINT heap_count = 1;
			ID3D12DescriptorHeap* sampler_heap = nullptr;
			if (command_buffer->sampler_memory_descriptor) {
				sampler_heap = *static_cast<DX_MEMORY_DESCRIPTOR*>(command_buffer->sampler_memory_descriptor);
				heap_count++;
			}
			ID3D12DescriptorHeap* heaps[] =
			{
				resource_heap,
				sampler_heap
			};
			i_command_buffer->SetDescriptorHeaps(heap_count, heaps);

			static float clearColor[] = { 0.1f, 0.2f, 0.4f, 1.0f };

			const DX_MEMORY_DESCRIPTOR_SLOT* rtv_slot = static_cast<const DX_MEMORY_DESCRIPTOR_SLOT*>(render_target_view_impl->memory_descriptor);
			const D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle = { rtv_slot->cpu_handle };
			i_command_buffer->RSSetViewports(1, &dx_vp);
			i_command_buffer->RSSetScissorRects(1, &dx_scissor);
			i_command_buffer->OMSetRenderTargets(1, &rtv_handle, FALSE, dsv_handle_ptr);
			i_command_buffer->ClearRenderTargetView(
				rtv_handle,
				clearColor,
				0,
				nullptr
			);

			if (dsv_handle_ptr != nullptr) {
				i_command_buffer->ClearDepthStencilView(
					*dsv_handle_ptr,                         // D3D12_CPU_DESCRIPTOR_HANDLE
					D3D12_CLEAR_FLAG_DEPTH,            // qué limpiar
					1.0f,                              // depth clear value
					0,                                 // stencil clear value
					0,                                 // num rects
					nullptr                            // rects
				);
			}
			
			if (pipeline_impl) {				
				
				D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle = 
					{ static_cast<DX_MEMORY_DESCRIPTOR*>(command_buffer->buffer_memory_descriptor)->gpu_handle };
				ID3D12PipelineState* i_pipeline = *pipeline_impl;
				ASSERT_PTR(i_pipeline);
				i_command_buffer->SetPipelineState(i_pipeline);
				ID3D12RootSignature* i_signature = *static_cast<DX_PIPELINE_LAYOUT*>(pipeline_impl->layout);
				ASSERT_PTR(i_signature);
				i_command_buffer->SetGraphicsRootSignature(i_signature);
				i_command_buffer->SetGraphicsRootDescriptorTable(0, gpu_handle);
				if (sampler_heap) {
					gpu_handle = { static_cast<DX_MEMORY_DESCRIPTOR*>(command_buffer->sampler_memory_descriptor)->gpu_handle };
					i_command_buffer->SetGraphicsRootDescriptorTable(1, gpu_handle);
				}
			}

			if (callback)
				callback();

	});

	dx12_command_buffer_resource_barrier_transition(i_command_buffer,
		{ resource_impl },
		{ D3D12_RESOURCE_STATE_PRESENT },
		nullptr);
}
