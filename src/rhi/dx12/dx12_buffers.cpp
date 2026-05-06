#include "dx12_buffers.hpp"
#include "dx12_command_buffer.hpp"
#include "dx12_command_queue.hpp"

std::unique_ptr<RHI_BUFFER> dx12_buffers_create_raw(const RHI_BUFFER_DESC& desc) {

	D3D12_RESOURCE_FLAGS  flags = D3D12_RESOURCE_FLAG_NONE;
	
	std::unique_ptr<D3D12_CLEAR_VALUE> clear_value;

	if (desc.format >= resource_format_d32_float_s8_uint
		&& desc.format <= resource_format_d16_norm) {

		flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		clear_value = std::make_unique<D3D12_CLEAR_VALUE>();
		clear_value->Format = dx12_resource_format_type[(int)desc.format];
		clear_value->DepthStencil.Depth = 1.0f;
		clear_value->DepthStencil.Stencil = 0;
	}
	else {
		for (auto f : buffer_resource_flags_type)
			flags |= ((int)desc.flags & (int)f) ? f : D3D12_RESOURCE_FLAG_NONE;
	}
	
	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = dx12_heap_type[desc.memory_type];
	heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProps.CreationNodeMask = 1;
	heapProps.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC bufferDesc = {};
	bufferDesc.Dimension = dx12_buffer_type[(int)desc.type];
	bufferDesc.Alignment = 0;
	bufferDesc.Width = static_cast<UINT>(desc.width);
	bufferDesc.Height = static_cast<UINT>(desc.height);
	bufferDesc.DepthOrArraySize = 1;
	bufferDesc.MipLevels = 1;
	bufferDesc.SampleDesc.Count = 1;
	bufferDesc.SampleDesc.Quality = 0;
	if (bufferDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
		bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	else
		bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	bufferDesc.Flags = flags;
	bufferDesc.Format = (bufferDesc.Dimension > D3D12_RESOURCE_DIMENSION_BUFFER)
		? dx12_resource_format_type[(int)desc.format]
		: DXGI_FORMAT_UNKNOWN;
	ID3D12Resource* i_resource = nullptr;
	ID3D12Device* i_device = static_cast<ID3D12Device*>(desc.device.get());
	HRESULT hr = i_device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_COMMON,
		clear_value.get(),
		IID_PPV_ARGS(&i_resource)
	);
	if (FAILED(hr) || !i_resource) {
		throw std::exception("Failed to create D3D12 resource");
	}
	return std::make_unique<DX_BUFFER>(i_resource, desc.width, desc.height);
}

std::unique_ptr<RHI_DEPTH_BUFFER> dx12_buffers_create_depth(const RHI_DEPTH_BUFFER_DESC& desc) {
	
	if (desc.pool == nullptr) {
		throw std::exception("Memory pool required.");
	}
	// overwrite desc to match must have depth buffer requeriments
	RHI_DEPTH_BUFFER_DESC db_desc_mutable = const_cast<RHI_DEPTH_BUFFER_DESC&>(desc);
	db_desc_mutable.memory_type = buffer_memory_type_default;
	db_desc_mutable.initial_state = resource_state_depth_write;
	db_desc_mutable.type = buffer_type_image_2d;
	if (db_desc_mutable.format < resource_format_d32_float_s8_uint
		|| db_desc_mutable.format > resource_format_d16_norm)
		throw std::exception("Invalid depth buffer format");
	auto depth_buffer = dx12_buffers_create_raw(db_desc_mutable);

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = dx12_resource_format_type[(int)db_desc_mutable.format];
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

	ID3D12Resource* i_resource = static_cast<ID3D12Resource*>(*depth_buffer.get());
	i_resource->AddRef();
	ID3D12Device* i_device = static_cast<ID3D12Device*>(desc.device.get());
	ID3D12DescriptorHeap* i_heap = static_cast<ID3D12DescriptorHeap*>(*desc.pool);
	std::unique_ptr<D3D12_CPU_DESCRIPTOR_HANDLE> dsv_handle = dx12_helpers_get_descriptor_heap_handle(i_device, i_heap, desc.slot);
	i_device->CreateDepthStencilView(i_resource, &dsvDesc, *dsv_handle);
	D3D12_RESOURCE_DESC r_desc = i_resource->GetDesc();
	return std::make_unique<DX_DEPTH_BUFFER>(i_resource, *dsv_handle, static_cast<size_t>(r_desc.Width), static_cast<size_t>(r_desc.Height));
}

void dx12_buffers_copy_buffer(RHI_COMMAND_BUFFER& command_buffer, RHI_BUFFER& src_buffer, RHI_BUFFER& dest_buffer) {

	ID3D12GraphicsCommandList* i_cmd_list = reinterpret_cast<ID3D12GraphicsCommandList*>(static_cast<ID3D12CommandList*>(command_buffer));
	ID3D12Resource* i_dest_buffer = static_cast<ID3D12Resource*>(dest_buffer);
	ID3D12Resource* i_src_buffer = static_cast<ID3D12Resource*>(src_buffer);
	i_cmd_list->CopyResource(i_dest_buffer, i_src_buffer);
}

void dx12_buffers_copy_buffer_region(RHI_COMMAND_BUFFER& command_buffer, RHI_BUFFER& src_buffer,
										size_t offset_src, RHI_BUFFER& dest_buffer,
										size_t offset_dest, size_t length){

	ID3D12GraphicsCommandList* i_cmd_list = reinterpret_cast<ID3D12GraphicsCommandList*>(static_cast<ID3D12CommandList*>(command_buffer));
	ID3D12Resource* i_dest_buffer = static_cast<ID3D12Resource*>(dest_buffer);
	ID3D12Resource* i_src_buffer = static_cast<ID3D12Resource*>(src_buffer);
	i_cmd_list->CopyBufferRegion(i_dest_buffer, offset_dest, 
									i_src_buffer, offset_src, length);
}

void dx12_buffers_gpu_write_region_with_states(RHI_COMMAND_BUFFER& command_buffer, RHI_BUFFER& cpu_buffer,
												RHI_BUFFER& gpu_buffer, size_t offset_src,
												size_t offset_dest, size_t length,
												resource_state prev_copy, resource_state after_copy) {

	ID3D12GraphicsCommandList* i_cmd_list = reinterpret_cast<ID3D12GraphicsCommandList*>(static_cast<ID3D12CommandList*>(command_buffer));
	ID3D12Resource* i_dest_buffer = static_cast<ID3D12Resource*>(gpu_buffer);
	
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = i_dest_buffer;
	barrier.Transition.StateBefore = dx12_resource_state_type[(int)prev_copy];
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	i_cmd_list->ResourceBarrier(1, &barrier);
	dx12_buffers_copy_buffer_region(command_buffer, cpu_buffer,
		offset_src, gpu_buffer, offset_dest, length);
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = dx12_resource_state_type[(int)after_copy];
	i_cmd_list->ResourceBarrier(1, &barrier);
	gpu_buffer.set_current_state(resource_state_constant_buffer);
}

void dx12_buffers_gpu_write_with_states(RHI_COMMAND_BUFFER& command_buffer, RHI_BUFFER& cpu_buffer,
										RHI_BUFFER& gpu_buffer, resource_state prev_copy, 
										resource_state after_copy) {

	ID3D12GraphicsCommandList* i_cmd_list = reinterpret_cast<ID3D12GraphicsCommandList*>(static_cast<ID3D12CommandList*>(command_buffer));
	ID3D12Resource* i_dest_buffer = static_cast<ID3D12Resource*>(gpu_buffer);

	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = i_dest_buffer;
	barrier.Transition.StateBefore = dx12_resource_state_type[(int)prev_copy];
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	i_cmd_list->ResourceBarrier(1, &barrier);
	dx12_buffers_copy_buffer(command_buffer, cpu_buffer,
		gpu_buffer);
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = dx12_resource_state_type[(int)after_copy];
	i_cmd_list->ResourceBarrier(1, &barrier);
	gpu_buffer.set_current_state(resource_state_constant_buffer);
}

void dx12_buffers_gpu_write_region(RHI_COMMAND_BUFFER& command_buffer, RHI_BUFFER& cpu_buffer,
									RHI_BUFFER& gpu_buffer, size_t offset_src, 
									size_t offset_dest, size_t length) {

	dx12_buffers_gpu_write_region_with_states(command_buffer, cpu_buffer,
												gpu_buffer, offset_src, 
												offset_dest, length,
												reinterpret_cast<RHI_BUFFER&>(gpu_buffer).get_current_state(), 
												resource_state_copy_src);
}
void dx12_buffers_gpu_write(RHI_COMMAND_BUFFER& command_buffer, RHI_BUFFER& cpu_buffer,
							RHI_BUFFER& gpu_buffer) {

	dx12_buffers_gpu_write_with_states(command_buffer, cpu_buffer,
										gpu_buffer,
										reinterpret_cast<RHI_BUFFER&>(gpu_buffer).get_current_state(),
										resource_state_copy_src);
}


void dx12_buffers_gpu_read_region_with_states(RHI_COMMAND_BUFFER& command_buffer, RHI_BUFFER& cpu_buffer,
												RHI_BUFFER& gpu_buffer, size_t offset_src,
												size_t offset_dest, size_t length,
												resource_state prev_copy, resource_state after_copy) {

	ID3D12GraphicsCommandList* i_cmd_list = reinterpret_cast<ID3D12GraphicsCommandList*>(static_cast<ID3D12CommandList*>(command_buffer));
	ID3D12Resource* i_src_buffer = static_cast<ID3D12Resource*>(gpu_buffer);

	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = i_src_buffer;
	barrier.Transition.StateBefore = dx12_resource_state_type[(int)prev_copy];
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	i_cmd_list->ResourceBarrier(1, &barrier);
	dx12_buffers_copy_buffer_region(command_buffer, gpu_buffer,
		offset_src, cpu_buffer, offset_dest, length);
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
	barrier.Transition.StateAfter = dx12_resource_state_type[(int)after_copy];
	i_cmd_list->ResourceBarrier(1, &barrier);
	gpu_buffer.set_current_state(resource_state_constant_buffer);
}

void dx12_buffers_gpu_read_with_states(RHI_COMMAND_BUFFER& command_buffer, RHI_BUFFER& cpu_buffer,
										RHI_BUFFER& gpu_buffer,	resource_state prev_copy,
										resource_state after_copy) {

	ID3D12GraphicsCommandList* i_cmd_list = reinterpret_cast<ID3D12GraphicsCommandList*>(static_cast<ID3D12CommandList*>(command_buffer));
	ID3D12Resource* i_src_buffer = static_cast<ID3D12Resource*>(gpu_buffer);

	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = i_src_buffer;
	barrier.Transition.StateBefore = dx12_resource_state_type[(int)prev_copy];
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	i_cmd_list->ResourceBarrier(1, &barrier);
	dx12_buffers_copy_buffer(command_buffer, gpu_buffer, cpu_buffer);
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
	barrier.Transition.StateAfter = dx12_resource_state_type[(int)after_copy];
	i_cmd_list->ResourceBarrier(1, &barrier);
	gpu_buffer.set_current_state(resource_state_constant_buffer);
}

void dx12_buffers_gpu_read_region(RHI_COMMAND_BUFFER& command_buffer, RHI_BUFFER& cpu_buffer,
									RHI_BUFFER& gpu_buffer, size_t offset_src, 
									size_t offset_dest, size_t length) {
	
	dx12_buffers_gpu_read_region_with_states(command_buffer, cpu_buffer,
											gpu_buffer, offset_src, 
											offset_dest, length,
											resource_state_copy_src, resource_state_constant_buffer);
}

void dx12_buffers_gpu_read(RHI_COMMAND_BUFFER& command_buffer, RHI_BUFFER& cpu_buffer, RHI_BUFFER& gpu_buffer) {

	dx12_buffers_gpu_read_with_states(command_buffer, cpu_buffer,
										gpu_buffer, resource_state_copy_src, 
										resource_state_constant_buffer);
}

void dx12_buffers_cpu_write(RHI_BUFFER& cpu_buffer, 
							RHI_VOID_PTR data, size_t offset,
							size_t length) {

	ID3D12Resource* i_cpu_buffer = static_cast<ID3D12Resource*>(cpu_buffer);
	D3D12_RANGE range{
		.Begin = offset,
		.End = length
	};
	void* mapped;
	if (FAILED(i_cpu_buffer->Map(0, &range, &mapped))) {
		throw std::exception("Error cannot map resource");
	}
	memcpy(mapped, data, length);
	i_cpu_buffer->Unmap(0, &range);
}

void dx12_buffers_cpu_read(RHI_BUFFER& cpu_buffer, RHI_VOID_PTR& data,
							size_t offset, size_t length) {

	ID3D12Resource* i_cpu_buffer = static_cast<ID3D12Resource*>(cpu_buffer);
	D3D12_RANGE range{
		.Begin = offset,
		.End = length
	};
	void* mapped;
	if (FAILED(i_cpu_buffer->Map(0, &range, &mapped))) {
		throw std::exception("Error cannot map resource");
	}
	memcpy(data, mapped, length);
	i_cpu_buffer->Unmap(0, &range);
}