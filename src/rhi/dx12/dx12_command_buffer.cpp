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
	
	if (ib != nullptr) {
		ID3D12Resource* i_ib = *ib;
		D3D12_INDEX_BUFFER_VIEW ib_view;
		ib_view.BufferLocation = i_ib->GetGPUVirtualAddress();
		ib_view.SizeInBytes = static_cast<UINT>(ib->get_length());
		ib_view.Format = dx12_resource_format_type[(int)ib->get_format()];
		i_command_buffer->IASetIndexBuffer(&ib_view);
		auto index_count = static_cast<UINT>(ib_view.SizeInBytes / ib->get_stride());
		i_command_buffer->DrawIndexedInstanced(index_count, 1, 0, 0, 0);
	}
	else {
		i_command_buffer->DrawInstanced(vb_view.SizeInBytes / vb_view.StrideInBytes, 1, 0, 0);
	}
}