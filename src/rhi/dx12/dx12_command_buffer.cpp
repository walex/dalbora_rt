#include "dx12_command_buffer.hpp"

RHI_COMMAND_BUFFER* dx12_command_buffer_create(
	const RHI_COMMAND_BUFFER_DESC* const desc, 
	D3D12_COMMAND_LIST_TYPE type) {

	ASSERT_PTR(desc);

	ID3D12Device* i_device = *static_cast<DX_DEVICE*>(desc->device);
	ASSERT_PTR(i_device);

	// Create command allocator
	ID3D12CommandAllocator* i_cmd_allocator = nullptr;
	ASSERT_SUCCESS(i_device->CreateCommandAllocator(type, IID_PPV_ARGS(&i_cmd_allocator)));
	ASSERT_PTR(i_cmd_allocator);

	// Create command list
	ID3D12GraphicsCommandList* i_cmd_list = nullptr;
	ASSERT_SUCCESS(i_device->CreateCommandList(0, type, i_cmd_allocator, nullptr, IID_PPV_ARGS(&i_cmd_list)));
	ASSERT_PTR(i_cmd_list);
	// Close the command list as it is created in an open state
	i_cmd_list->Close();
	// Wrap the command list
	DX_COMMAND_BUFFER* result = new DX_COMMAND_BUFFER();
	ASSERT_PTR(result);
	result->set_handle(i_cmd_list);
	result->allocator = i_cmd_allocator;
	return result;
}

RHI_COMMAND_BUFFER* dx12_command_buffer_create_for_copy(
	const RHI_COMMAND_BUFFER_DESC* const desc) {
	return dx12_command_buffer_create(desc, D3D12_COMMAND_LIST_TYPE_COPY);
}
RHI_COMMAND_BUFFER* dx12_command_buffer_create_for_compute(
	const RHI_COMMAND_BUFFER_DESC* const desc) {
	return dx12_command_buffer_create(desc, D3D12_COMMAND_LIST_TYPE_COMPUTE);
}
RHI_COMMAND_BUFFER* dx12_command_buffer_create_for_render(
	const RHI_COMMAND_BUFFER_DESC* const desc) {
	return dx12_command_buffer_create(desc, D3D12_COMMAND_LIST_TYPE_DIRECT);
}
void dx12_command_buffer_record(
	RHI_COMMAND_BUFFER* const command_buffer,
	fptr_command_buffer_on_record callback) {

	ASSERT_PTR(command_buffer);

	DX_COMMAND_BUFFER* cmd_buffer_impl = static_cast<DX_COMMAND_BUFFER*>(command_buffer);
	ASSERT_PTR(cmd_buffer_impl->allocator);

	ID3D12GraphicsCommandList* i_cmd_list = *cmd_buffer_impl;
	ASSERT_PTR(i_cmd_list);

	ID3D12CommandAllocator* i_cmd_alloc = cmd_buffer_impl->allocator.Get();
	ASSERT_PTR(i_cmd_alloc);

	i_cmd_alloc->Reset();
	i_cmd_list->Reset(i_cmd_alloc, nullptr);

	callback(static_cast<RHI_VOID_PTR>(i_cmd_list));

	i_cmd_list->Close();
}

void dx12_command_buffer_draw_triangle_list(
	RHI_COMMAND_BUFFER* const command_buffer, 
	RHI_BUFFER* const vb,
	RHI_BUFFER* const ib) {

	ASSERT_PTR(command_buffer);
	ASSERT_PTR(vb);

	DX_COMMAND_BUFFER* cmd_buffer_impl = static_cast<DX_COMMAND_BUFFER*>(command_buffer);
	ID3D12GraphicsCommandList* i_command_buffer = *cmd_buffer_impl;
	ASSERT_PTR(i_command_buffer);
	
	DX_BUFFER* vb_impl = static_cast<DX_BUFFER*>(vb);
	ID3D12Resource* i_vb = *vb_impl;
	ASSERT_PTR(i_vb);

	DX_BUFFER* ib_impl = static_cast<DX_BUFFER*>(ib);

	D3D12_VERTEX_BUFFER_VIEW vb_view;
	vb_view.BufferLocation = i_vb->GetGPUVirtualAddress();
	vb_view.SizeInBytes = static_cast<UINT>(vb_impl->length);
	vb_view.StrideInBytes = static_cast<UINT>(vb_impl->stride);
	
	i_command_buffer->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	i_command_buffer->IASetVertexBuffers(0, 1, &vb_view);
	
	std::vector<DX_RESOURCE*> resources;
	resources.reserve(2);
	resources.push_back(vb_impl);
	if (ib_impl != nullptr) {
		resources.push_back(ib_impl);
	}
	dx12_command_buffer_resource_barrier_transition(i_command_buffer,
		resources,
		{D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,D3D12_RESOURCE_STATE_INDEX_BUFFER },
		[&]() {
		
			if (ib_impl != nullptr) {
				
				ID3D12Resource* i_ib = *ib_impl;
				ASSERT_PTR(i_ib);
				D3D12_INDEX_BUFFER_VIEW ib_view;
				ib_view.BufferLocation = i_ib->GetGPUVirtualAddress();
				ib_view.SizeInBytes = static_cast<UINT>(ib_impl->length);
				ib_view.Format = dx12_resource_format_type[ib_impl->format];
				i_command_buffer->IASetIndexBuffer(&ib_view);
				UINT index_count = static_cast<UINT>(ib_view.SizeInBytes / ib->stride);
				i_command_buffer->DrawIndexedInstanced(index_count, 1, 0, 0, 0);
			}
			else {
				i_command_buffer->DrawInstanced(vb_view.SizeInBytes / vb_view.StrideInBytes, 1, 0, 0);
			}
		});
}

void dx12_command_buffer_ray_trace(
	RHI_COMMAND_BUFFER* const command_buffer, 
	RHI_TEXTURE_2D* const render_target,
	const RHI_SBT_TABLE* const sbt) {

	ASSERT_PTR(command_buffer);
	ASSERT_PTR(render_target);

	ASSERT_PTR(sbt);

	DX_COMMAND_BUFFER* cmd_buffer_impl = static_cast<DX_COMMAND_BUFFER*>(command_buffer);
	ID3D12GraphicsCommandList* i_command_buffer_0 = *cmd_buffer_impl;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList5> i_command_buffer;
	ASSERT_SUCCESS(i_command_buffer_0->QueryInterface(IID_PPV_ARGS(&i_command_buffer)));
	ASSERT_PTR(i_command_buffer);	
	DX_RESOURCE* render_target_impl = static_cast<DX_TEXTURE_2D*>(render_target);
	ID3D12Resource* i_table = *static_cast<const DX_SBT_TABLE*>(sbt);	
	ASSERT_PTR(i_table);

	dx12_command_buffer_resource_barrier_transition_and_restore(i_command_buffer.Get(),
		{ render_target_impl },
		{ D3D12_RESOURCE_STATE_UNORDERED_ACCESS },
		[&]() {

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
		
			D3D12_RESOURCE_BARRIER uav_barrier = {};
			uav_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
			uav_barrier.UAV.pResource = *render_target_impl;
			i_command_buffer->ResourceBarrier(1, &uav_barrier);
		});
}

void dx12_command_buffer_copy_texture(
	RHI_COMMAND_BUFFER* const command_buffer, 
	RHI_TEXTURE_2D* const dest_texture,
	const RHI_TEXTURE_2D* const src_texture) {

	ASSERT_PTR(command_buffer);
	ASSERT_PTR(dest_texture);
	ASSERT_PTR(src_texture);

	ID3D12GraphicsCommandList* i_command_buffer = *static_cast<DX_COMMAND_BUFFER*>(command_buffer);
	ASSERT_PTR(i_command_buffer);
	DX_TEXTURE_2D* dest_texture_impl = static_cast<DX_TEXTURE_2D*>(dest_texture);
	DX_TEXTURE_2D* src_texture_impl = const_cast<DX_TEXTURE_2D*>(static_cast<const DX_TEXTURE_2D*>(src_texture));

	dx12_command_buffer_resource_barrier_transition_and_restore(i_command_buffer,
		{ src_texture_impl, dest_texture_impl },
		{D3D12_RESOURCE_STATE_COPY_SOURCE,D3D12_RESOURCE_STATE_COPY_DEST},
		[&]() {
			i_command_buffer->CopyResource(
				*dest_texture_impl,
				*src_texture_impl);
		});
}

void dx12_command_buffer_resource_barrier_transition(
	ID3D12GraphicsCommandList* const i_command_buffer,
	const std::vector<DX_RESOURCE*>& resources_impl,
	const std::vector<D3D12_RESOURCE_STATES>& states,
	std::function<void()> cb) {
	
	ASSERT_PTR(i_command_buffer);

	std::vector<D3D12_RESOURCE_BARRIER> barriers;
	barriers.reserve(resources_impl.size());
	for (size_t i = 0; i < resources_impl.size(); ++i) {

		D3D12_RESOURCE_STATES init_state = resources_impl[i]->current_state;
		D3D12_RESOURCE_STATES end_state = states[i];
		if (init_state == end_state) {
			continue;
		}
		D3D12_RESOURCE_BARRIER& barrier = barriers.emplace_back();
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = *resources_impl[i];
		ASSERT_PTR(barrier.Transition.pResource);
		barrier.Transition.StateBefore = init_state;
		barrier.Transition.StateAfter = end_state;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		resources_impl[i]->current_state.store(end_state);
	}
	if (barriers.size() > 0) {
		i_command_buffer->ResourceBarrier(static_cast<UINT>(barriers.size()), barriers.data());
	}
	if (cb)	cb();
}


void dx12_command_buffer_resource_barrier_transition_and_restore(
	ID3D12GraphicsCommandList* const i_command_buffer,
	const std::vector<DX_RESOURCE*>& resources_impl,
	const std::vector<D3D12_RESOURCE_STATES>& states,
	std::function<void()> cb) {

	ASSERT_PTR(i_command_buffer);

	std::vector<D3D12_RESOURCE_BARRIER> barriers;
	std::vector<D3D12_RESOURCE_STATES> prev_states;
	barriers.reserve(resources_impl.size());
	prev_states.reserve(resources_impl.size());
	for (size_t i = 0; i < resources_impl.size(); ++i) {

		D3D12_RESOURCE_STATES init_state = resources_impl[i]->current_state;
		D3D12_RESOURCE_STATES end_state = states[i];
		if (init_state == end_state) {
			continue;
		}
		prev_states.push_back(init_state);
		D3D12_RESOURCE_BARRIER& barrier = barriers.emplace_back();
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = *resources_impl[i];
		ASSERT_PTR(barrier.Transition.pResource);
		barrier.Transition.StateBefore = init_state;
		barrier.Transition.StateAfter = end_state;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		resources_impl[i]->current_state.store(end_state);
	}
	if (barriers.size() > 0) {		
		i_command_buffer->ResourceBarrier(static_cast<UINT>(barriers.size()), barriers.data());
		if (cb)	cb();
		for (size_t i = 0; i < barriers.size(); ++i) {
			D3D12_RESOURCE_BARRIER& barrier = barriers.at(i);
			barrier.Transition.StateBefore = barrier.Transition.StateAfter;
			barrier.Transition.StateAfter = prev_states.at(i);
			resources_impl[i]->current_state.store(barrier.Transition.StateAfter);
		}
		i_command_buffer->ResourceBarrier(static_cast<UINT>(barriers.size()), barriers.data());
	}
	else if (cb) { cb();	}
}

