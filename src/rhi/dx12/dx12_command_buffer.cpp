#include "dx12_command_buffer.hpp"

RHI_COMMAND_BUFFER* dx12_command_buffer_create(const RHI_COMMAND_BUFFER_DESC* const desc, D3D12_COMMAND_LIST_TYPE type) {

	ASSERT_NULL(desc);

	ID3D12Device* i_device = *static_cast<DX_DEVICE*>(desc->device);
	ASSERT_NULL(i_device);

	// Create command allocator
	ID3D12CommandAllocator* i_cmd_allocator = nullptr;
	ASSERT_FAILED(i_device->CreateCommandAllocator(type, IID_PPV_ARGS(&i_cmd_allocator)));
	ASSERT_NULL(i_cmd_allocator);

	// Create command list
	ID3D12GraphicsCommandList* i_cmd_list = nullptr;
	ASSERT_FAILED(i_device->CreateCommandList(0, type, i_cmd_allocator, nullptr, IID_PPV_ARGS(&i_cmd_list)));
	ASSERT_NULL(i_cmd_list);
	// Close the command list as it is created in an open state
	i_cmd_list->Close();
	// Wrap the command list
	DX_COMMAND_BUFFER* result = new DX_COMMAND_BUFFER();
	ASSERT_NULL(result);
	result->set_handle(i_cmd_list);
	result->allocator = i_cmd_allocator;
	return result;
}

RHI_COMMAND_BUFFER* dx12_command_buffer_create_for_copy(const RHI_COMMAND_BUFFER_DESC* const desc) {
	return dx12_command_buffer_create(desc, D3D12_COMMAND_LIST_TYPE_COPY);
}
RHI_COMMAND_BUFFER* dx12_command_buffer_create_for_compute(const RHI_COMMAND_BUFFER_DESC* const desc) {
	return dx12_command_buffer_create(desc, D3D12_COMMAND_LIST_TYPE_COMPUTE);
}
RHI_COMMAND_BUFFER* dx12_command_buffer_create_for_render(const RHI_COMMAND_BUFFER_DESC* const desc) {
	return dx12_command_buffer_create(desc, D3D12_COMMAND_LIST_TYPE_DIRECT);
}
void dx12_command_buffer_record(RHI_COMMAND_BUFFER* const command_buffer,
								fptr_command_buffer_on_record callback) {

	ASSERT_NULL(command_buffer);

	DX_COMMAND_BUFFER* cmd_buffer_impl = static_cast<DX_COMMAND_BUFFER*>(command_buffer);
	ASSERT_NULL(cmd_buffer_impl->allocator);

	ID3D12GraphicsCommandList* i_cmd_list = *cmd_buffer_impl;
	ASSERT_NULL(i_cmd_list);

	ID3D12CommandAllocator* i_cmd_alloc = cmd_buffer_impl->allocator.Get();
	ASSERT_NULL(i_cmd_alloc);

	i_cmd_alloc->Reset();
	i_cmd_list->Reset(i_cmd_alloc, nullptr);

	callback(static_cast<RHI_VOID_PTR>(i_cmd_list));

	i_cmd_list->Close();
}

void dx12_command_buffer_draw_triangle_list(RHI_COMMAND_BUFFER* const command_buffer, const RHI_VERTEX_BUFFER* const vb,
	const RHI_INDEX_BUFFER* const ib) {

	ASSERT_NULL(command_buffer);
	ASSERT_NULL(vb);

	DX_COMMAND_BUFFER* cmd_buffer_impl = static_cast<DX_COMMAND_BUFFER*>(command_buffer);
	ID3D12GraphicsCommandList* i_command_buffer = *cmd_buffer_impl;
	ASSERT_NULL(i_command_buffer);
	
	DX_VERTEX_BUFFER* vb_impl = *static_cast<const DX_VERTEX_BUFFER*>(vb);
	ID3D12Resource* i_vb = *vb_impl;
	ASSERT_NULL(i_vb);

	D3D12_VERTEX_BUFFER_VIEW vb_view;
	vb_view.BufferLocation = i_vb->GetGPUVirtualAddress();
	vb_view.SizeInBytes = static_cast<UINT>(vb->length);
	vb_view.StrideInBytes = static_cast<UINT>(vb->stride);
	
	i_command_buffer->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	i_command_buffer->IASetVertexBuffers(0, 1, &vb_view);
	
	dx12_command_buffer_resource_transition_block(i_command_buffer,
		vb_impl,
		D3D12_RESOURCE_STATE_COMMON,
		true, [&]() {

			if (ib != nullptr) {

				DX_VERTEX_BUFFER* ib_impl = *static_cast<const DX_INDEX_BUFFER*>(ib);
				ID3D12Resource* i_ib = *ib_impl;
				ASSERT_NULL(i_ib);

				dx12_command_buffer_resource_transition_block(i_command_buffer, 
					ib_impl,
					D3D12_RESOURCE_STATE_COMMON,
					true, [&]() {

						D3D12_INDEX_BUFFER_VIEW ib_view;
						ib_view.BufferLocation = i_ib->GetGPUVirtualAddress();
						ib_view.SizeInBytes = static_cast<UINT>(ib->length);
						ib_view.Format = dx12_resource_format_type[ib->format];
						i_command_buffer->IASetIndexBuffer(&ib_view);
						auto index_count = static_cast<UINT>(ib_view.SizeInBytes / ib->stride);
						i_command_buffer->DrawIndexedInstanced(index_count, 1, 0, 0, 0);
					});
			}
			else {
				i_command_buffer->DrawInstanced(vb_view.SizeInBytes / vb_view.StrideInBytes, 1, 0, 0);
			}
		});
}

void dx12_command_buffer_ray_trace(RHI_COMMAND_BUFFER* const command_buffer, 
	RHI_TEXTURE_2D* const render_target, const RHI_RT_PIPELINE* const pipeline, 
	const RHI_BUFFER* const bvh_instances, const RHI_SBT_TABLE* const sbt) {

	ASSERT_NULL(command_buffer);
	ASSERT_NULL(render_target);
	ASSERT_NULL(pipeline);
	ASSERT_NULL(bvh_instances);
	ASSERT_NULL(sbt);

	DX_COMMAND_BUFFER* cmd_buffer_impl = static_cast<DX_COMMAND_BUFFER*>(command_buffer);
	ID3D12GraphicsCommandList* i_command_buffer_0 = *cmd_buffer_impl;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList5> i_command_buffer;
	ASSERT_FAILED(i_command_buffer_0->QueryInterface(IID_PPV_ARGS(&i_command_buffer)));
	ASSERT_NULL(i_command_buffer);	
	DX_TEXTURE_2D* render_target_impl = static_cast<DX_TEXTURE_2D*>(render_target);
	ID3D12Resource* i_table = *static_cast<const DX_SBT_BUFFER*>(sbt);	
	ASSERT_NULL(i_table);

	dx12_command_buffer_resource_transition_block(i_command_buffer.Get(), 
		render_target_impl,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		true, [&]() {

			D3D12_DISPATCH_RAYS_DESC desc = {};
			desc.RayGenerationShaderRecord.StartAddress =
				i_table->GetGPUVirtualAddress() + sbt->ray_gen_offset;
			desc.RayGenerationShaderRecord.SizeInBytes =
				sbt->ray_gen_size;
			desc.MissShaderTable.StartAddress =
				i_table->GetGPUVirtualAddress() + sbt->miss_offset;
			desc.MissShaderTable.SizeInBytes =
				sbt->miss_size;
			desc.MissShaderTable.StrideInBytes =
				sbt->record_size;
			desc.HitGroupTable.StartAddress =
				i_table->GetGPUVirtualAddress() + sbt->hit_group_offset;
			desc.HitGroupTable.StrideInBytes =
				sbt->record_size;
			desc.HitGroupTable.SizeInBytes = sbt->hit_group_size;
			desc.Width = static_cast<UINT>(render_target->width);
			desc.Height = static_cast<UINT>(render_target->height);
			desc.Depth = 1;
			i_command_buffer->DispatchRays(&desc);
		});
}

void dx12_command_buffer_copy_texture(RHI_COMMAND_BUFFER* const command_buffer, RHI_TEXTURE_2D* const dest_texture,
	const RHI_TEXTURE_2D* const src_texture) {

	ASSERT_NULL(command_buffer);
	ASSERT_NULL(dest_texture);
	ASSERT_NULL(src_texture);

	ID3D12GraphicsCommandList* i_command_buffer = *static_cast<DX_COMMAND_BUFFER*>(command_buffer);
	ASSERT_NULL(i_command_buffer);
	DX_TEXTURE_2D* dest_texture_impl = static_cast<DX_TEXTURE_2D*>(dest_texture);
	DX_TEXTURE_2D* src_texture_impl = const_cast<DX_TEXTURE_2D*>(static_cast<const DX_TEXTURE_2D*>(src_texture));
	
	dx12_command_buffer_resource_transition_block(i_command_buffer, 
		src_texture_impl,
		D3D12_RESOURCE_STATE_COPY_SOURCE,
		true, [&]() {

			dx12_command_buffer_resource_transition_block(i_command_buffer, 
				dest_texture_impl,
				D3D12_RESOURCE_STATE_COPY_DEST,
				true, [&]() {

					i_command_buffer->CopyResource(
						*dest_texture_impl,
						*src_texture_impl
					);
				});
		});
}

void dx12_command_buffer_resource_transition_block(ID3D12GraphicsCommandList* const i_command_buffer,
	DX_RESOURCE* const resource_impl,
	D3D12_RESOURCE_STATES end_state,
	bool restore, std::function<void()> cb) {

	ASSERT_NULL(i_command_buffer);
	ASSERT_NULL(resource_impl);

	ID3D12Resource* i_resource = *resource_impl;
	D3D12_RESOURCE_STATES init_state = resource_impl->current_state;
	if (init_state == end_state) {
		if (cb)	cb();
		return;
	}
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = i_resource;
	barrier.Transition.StateBefore = init_state;
	barrier.Transition.StateAfter = end_state;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	i_command_buffer->ResourceBarrier(1, &barrier);
	if (cb)	cb();
	if (restore == true) {
		barrier.Transition.pResource = i_resource;
		barrier.Transition.StateBefore = end_state;
		barrier.Transition.StateAfter = init_state;
		i_command_buffer->ResourceBarrier(1, &barrier);
	}
}