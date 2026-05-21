#include "dx12_command_buffer.hpp"
#include "dx12_command_queue.hpp"

std::unique_ptr<RHI_COMMAND_BUFFER> dx12_command_buffer_create(const RHI_COMMAND_BUFFER_DESC& desc, D3D12_COMMAND_LIST_TYPE type) {

	// For simplicity, we will create a command allocator and a command list
	ID3D12Device* i_device = desc.device.get();
	if (!i_device) {
		throw std::exception("Invalid device for command buffer creation");
	}
	// Create command allocator
	ID3D12CommandAllocator* i_cmd_allocator = nullptr;
	HRESULT hr = i_device->CreateCommandAllocator(type, IID_PPV_ARGS(&i_cmd_allocator));
	if (FAILED(hr) || !i_cmd_allocator) {
		throw std::exception("Failed to create D3D12 command allocator");
	}
	// Create command list
	ID3D12GraphicsCommandList* i_cmd_list = nullptr;
	hr = i_device->CreateCommandList(0, type, i_cmd_allocator, nullptr, IID_PPV_ARGS(&i_cmd_list));
	if (FAILED(hr) || !i_cmd_list) {
		if (i_cmd_allocator) i_cmd_allocator->Release();
		throw std::exception("Failed to create D3D12 command list");
	}
	// Close the command list as it is created in an open state
	i_cmd_list->Close();
	// Wrap the command list in a RHI_OBJECT
	return std::make_unique<DX_COMMAND_BUFFER>(i_cmd_list, i_cmd_allocator);
}

std::unique_ptr<RHI_COMMAND_BUFFER> dx12_command_buffer_create_for_copy(const RHI_COMMAND_BUFFER_DESC& desc) {
	return dx12_command_buffer_create(desc, D3D12_COMMAND_LIST_TYPE_COPY);
}
std::unique_ptr<RHI_COMMAND_BUFFER> dx12_command_buffer_create_for_compute(const RHI_COMMAND_BUFFER_DESC& desc) {
	return dx12_command_buffer_create(desc, D3D12_COMMAND_LIST_TYPE_COMPUTE);
}
std::unique_ptr<RHI_COMMAND_BUFFER> dx12_command_buffer_create_for_render(const RHI_COMMAND_BUFFER_DESC& desc) {
	return dx12_command_buffer_create(desc, D3D12_COMMAND_LIST_TYPE_DIRECT);
}
void dx12_command_buffer_record(RHI_COMMAND_BUFFER& command_buffer,
								fptr_command_buffer_on_record callback) {

	auto& cmd_buffer_impl = reinterpret_cast<DX_COMMAND_BUFFER&>(command_buffer);
	ID3D12GraphicsCommandList* i_cmd_list = cmd_buffer_impl;
	ID3D12CommandAllocator* i_cmd_alloc = cmd_buffer_impl;
	i_cmd_alloc->Reset();
	i_cmd_list->Reset(i_cmd_alloc, nullptr);

	callback(static_cast<RHI_VOID_PTR>(i_cmd_list));

	i_cmd_list->Close();
}

void dx12_command_buffer_reset_resource_state(RHI_COMMAND_BUFFER& command_buffer, RHI_RESOURCE& resource) {

	ID3D12GraphicsCommandList* i_command_buffer = command_buffer;
	static D3D12_RESOURCE_BARRIER resource_barrier = {
		.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION
	};
	ID3D12Resource* i_res = resource;
	if (resource.get_current_state() != resource.get_default_state()) {
		resource_barrier.Transition.pResource = i_res;
		resource_barrier.Transition.StateBefore = dx12_resource_state_type[(int)resource.get_current_state()];
		resource_barrier.Transition.StateAfter = dx12_resource_state_type[(int)resource.get_default_state()];
		resource_barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		i_command_buffer->ResourceBarrier(1, &resource_barrier);
		resource.set_current_state(resource.get_default_state());
	}
}

void dx12_command_buffer_draw_triangle_list(RHI_COMMAND_BUFFER& command_buffer, RHI_VERTEX_BUFFER& vb, 
											RHI_INDEX_BUFFER* ib) {

	ID3D12Resource* i_vb = vb;
	D3D12_VERTEX_BUFFER_VIEW vb_view;
	vb_view.BufferLocation = i_vb->GetGPUVirtualAddress();
	vb_view.SizeInBytes = static_cast<UINT>(vb.get_length());
	vb_view.StrideInBytes = static_cast<UINT>(vb.get_stride());
	ID3D12GraphicsCommandList* i_command_buffer = command_buffer;
	i_command_buffer->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	i_command_buffer->IASetVertexBuffers(0, 1, &vb_view);
	
	resource_state old_state_vb = vb.get_current_state();
	
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	
	if (old_state_vb != resource_state_none) {
		barrier.Transition.pResource = vb;
		barrier.Transition.StateBefore = dx12_resource_state_type[old_state_vb];
		i_command_buffer->ResourceBarrier(1, &barrier);
		vb.set_current_state(resource_state_none);
	}

	if (ib != nullptr) {
		resource_state old_state_ib = ib->get_current_state();
		if (old_state_ib != resource_state_none) {
			barrier.Transition.pResource = *ib;
			barrier.Transition.StateBefore = dx12_resource_state_type[old_state_ib];
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
			i_command_buffer->ResourceBarrier(1, &barrier);
			ib->set_current_state(resource_state_none);
		}
		ID3D12Resource* i_ib = *ib;
		D3D12_INDEX_BUFFER_VIEW ib_view;
		ib_view.BufferLocation = i_ib->GetGPUVirtualAddress();
		ib_view.SizeInBytes = static_cast<UINT>(ib->get_length());
		ib_view.Format = dx12_resource_format_type[(int)ib->get_format()];
		i_command_buffer->IASetIndexBuffer(&ib_view);
		auto index_count = static_cast<UINT>(ib_view.SizeInBytes / ib->get_stride());
		i_command_buffer->DrawIndexedInstanced(index_count, 1, 0, 0, 0);
		if (old_state_ib != resource_state_none) {
			barrier.Transition.pResource = *ib;
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
			barrier.Transition.StateAfter = dx12_resource_state_type[old_state_ib];
			i_command_buffer->ResourceBarrier(1, &barrier);
			ib->set_current_state(old_state_ib);
		}
	}
	else {
		i_command_buffer->DrawInstanced(vb_view.SizeInBytes / vb_view.StrideInBytes, 1, 0, 0);
	}
	if (old_state_vb != resource_state_none) {
		barrier.Transition.pResource = vb;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
		barrier.Transition.StateAfter = dx12_resource_state_type[old_state_vb];
		i_command_buffer->ResourceBarrier(1, &barrier);
		vb.set_current_state(old_state_vb);
	}
}

void dx12_command_buffer_ray_trace(RHI_DEVICE& device, RHI_COMMAND_BUFFER& command_buffer, 
	RHI_TEXTURE_2D& render_target, RHI_RT_PIPELINE& pipeline, 
	RHI_BUFFER& bvh_instances, RHI_BUFFER& sbt) {

	ID3D12GraphicsCommandList* i_command_buffer_0 = command_buffer;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList5> i_command_buffer;
	i_command_buffer_0->QueryInterface(IID_PPV_ARGS(&i_command_buffer));
	DX_DEVICE& device_impl = static_cast<DX_DEVICE&>(device);
	DX_SBT_BUFFER& sbt_impl = static_cast<DX_SBT_BUFFER&>(sbt);

	resource_state old_state_rt = render_target.get_current_state();
	D3D12_RESOURCE_BARRIER barrier = {};
	if (old_state_rt != resource_state_rt_render_target) {
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = render_target;
		barrier.Transition.StateBefore = dx12_resource_state_type[old_state_rt];
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		i_command_buffer->ResourceBarrier(1, &barrier);
		render_target.set_current_state(resource_state_rt_render_target);
	}
		
	const size_t shader_id_size = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
	ID3D12Resource* i_table = static_cast<DX_BUFFER&>(sbt);

	D3D12_DISPATCH_RAYS_DESC desc = {};

	desc.RayGenerationShaderRecord.StartAddress =
		i_table->GetGPUVirtualAddress() + sbt_impl.get_ray_gen_offset();
	desc.RayGenerationShaderRecord.SizeInBytes =
		sbt_impl.get_ray_gen_size();
	desc.MissShaderTable.StartAddress =
		i_table->GetGPUVirtualAddress() + sbt_impl.get_miss_offset();
	desc.MissShaderTable.SizeInBytes =
		sbt_impl.get_miss_size();
	desc.MissShaderTable.StrideInBytes =
		sbt_impl.get_record_size();
	desc.HitGroupTable.StartAddress =
		i_table->GetGPUVirtualAddress() + sbt_impl.get_hit_group_offset();
	desc.HitGroupTable.StrideInBytes =
		sbt_impl.get_record_size();
	desc.HitGroupTable.SizeInBytes = sbt_impl.get_hit_group_size();
	desc.Width = static_cast<UINT>(render_target.get_width());
	desc.Height = static_cast<UINT>(render_target.get_height());
	desc.Depth = 1;

	i_command_buffer->DispatchRays(&desc);

	D3D12_RESOURCE_BARRIER uav_barrier = {};
	uav_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	uav_barrier.UAV.pResource = render_target;
	i_command_buffer->ResourceBarrier(1, &uav_barrier);
	if (old_state_rt != resource_state_rt_render_target) {
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = render_target;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		barrier.Transition.StateAfter = dx12_resource_state_type[old_state_rt];
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		i_command_buffer->ResourceBarrier(1, &barrier);
		render_target.set_current_state(old_state_rt);
	}
}

void dx12_command_buffer_copy_texture(RHI_COMMAND_BUFFER& command_buffer, RHI_TEXTURE_2D& dest_texture, RHI_TEXTURE_2D& src_texture) {

	// src_texture:

	D3D12_RESOURCE_BARRIER barriers[2] = {};

	resource_state old_state_src = src_texture.get_current_state();
	barriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barriers[0].Transition.pResource = static_cast<DX_TEXTURE_2D&>(src_texture);
	barriers[0].Transition.StateBefore = dx12_resource_state_type[old_state_src];
	barriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
	barriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	// back buffer:

	resource_state old_state_dest = dest_texture.get_current_state();
	barriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barriers[1].Transition.pResource = static_cast<DX_TEXTURE_2D&>(dest_texture);
	barriers[1].Transition.StateBefore = dx12_resource_state_type[old_state_dest];
	barriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
	barriers[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	ID3D12GraphicsCommandList* i_command_buffer = command_buffer;

	i_command_buffer->ResourceBarrier(2, barriers);
	src_texture.set_current_state(resource_state_copy_src);
	dest_texture.set_current_state(resource_state_copy_dest);

	i_command_buffer->CopyResource(
		static_cast<DX_TEXTURE_2D&>(dest_texture),
		static_cast<DX_TEXTURE_2D&>(src_texture)
	);
	
	barriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
	barriers[0].Transition.StateAfter = dx12_resource_state_type[old_state_src];

	barriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barriers[1].Transition.StateAfter = dx12_resource_state_type[old_state_dest];

	i_command_buffer->ResourceBarrier(2, barriers);
	src_texture.set_current_state(old_state_src);
	dest_texture.set_current_state(old_state_dest);
}