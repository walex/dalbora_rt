#include "dx12_buffers.hpp"
#include "dx12_command_buffer.hpp"
#include "dx12_command_queue.hpp"

std::unique_ptr<RHI_BUFFER> dx12_buffers_create_2d(const RHI_BUFFER_2D_DESC &desc)
{

	D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;

	std::unique_ptr<D3D12_CLEAR_VALUE> clear_value;
	D3D12_RESOURCE_STATES resource_initial_state;
	buffer_type buffer_type = desc.type;
	if (buffer_type == buffer_type_depth_stencil)
	{

		flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		clear_value = std::make_unique<D3D12_CLEAR_VALUE>();
		clear_value->Format = dx12_resource_format_type[(int)desc.format];
		clear_value->DepthStencil.Depth = 1.0f;
		clear_value->DepthStencil.Stencil = 0;
		buffer_type = buffer_type_image_2d;
	}
	else if (buffer_type == buffer_type_rt_bvh
		|| desc.default_state == resource_state_rt_render_target) {
		flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	}

	if (desc.default_state == resource_state_rt_render_target
		|| desc.default_state == resource_state_raster_render_target) {

		flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	}
	resource_initial_state = dx12_resource_state_type[desc.default_state];
	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = dx12_heap_type[desc.memory_type];
	heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProps.CreationNodeMask = 1;
	heapProps.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC bufferDesc = {};
	bufferDesc.Dimension = dx12_buffer_type[(int)buffer_type];
	bufferDesc.Alignment = 0;
	bufferDesc.Width = static_cast<UINT>(desc.width);
	bufferDesc.Height = static_cast<UINT>(desc.height);
	bufferDesc.DepthOrArraySize = 1;
	bufferDesc.MipLevels = static_cast<UINT>(desc.mips);
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
	ID3D12Resource *i_resource = nullptr;
	ID3D12Device *i_device = desc.device.get();
	HRESULT hr = i_device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		resource_initial_state,
		clear_value.get(),
		IID_PPV_ARGS(&i_resource));
	if (FAILED(hr) || !i_resource)
	{
		throw std::exception("Failed to create D3D12 resource");
	}
	return std::make_unique<DX_BUFFER>(i_resource, desc.default_state, desc.format, desc.length);
}

std::unique_ptr<RHI_BUFFER> dx12_buffers_create_raw(const RHI_BUFFER_DESC &desc)
{
	RHI_BUFFER_2D_DESC desc_2d(desc.device);
	desc_2d.length = desc.length;
	desc_2d.mips = desc.mips;
	desc_2d.default_state = desc.default_state;
	desc_2d.memory_type = desc.memory_type;
	desc_2d.format = desc.format;
	desc_2d.type = desc.type;
	desc_2d.width = desc.length;
	desc_2d.height = 1;
	return dx12_buffers_create_2d(desc_2d);
}

std::unique_ptr<RHI_DEPTH_BUFFER> dx12_buffers_create_depth(const RHI_DEPTH_BUFFER_DESC &desc)
{

	DX_DEVICE &device_impl = reinterpret_cast<DX_DEVICE &>(desc.device.get());
	DX_HEAP *heap_impl = device_impl.get_dsv_heap();
	if (heap_impl == nullptr)
	{
		throw std::exception("NO heap found for dsv.");
	}
	// overwrite desc to match must have depth buffer requeriments
	RHI_DEPTH_BUFFER_DESC db_desc_mutable = const_cast<RHI_DEPTH_BUFFER_DESC &>(desc);
	if (db_desc_mutable.format < resource_format_d32_float_s8_uint || db_desc_mutable.format > resource_format_d16_norm)
		throw std::exception("Invalid depth buffer format");
	db_desc_mutable.memory_type = buffer_memory_type_default;
	db_desc_mutable.default_state = resource_state_depth_write;
	db_desc_mutable.type = buffer_type_depth_stencil;
	auto depth_buffer = dx12_buffers_create_2d(db_desc_mutable);

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = dx12_resource_format_type[(int)db_desc_mutable.format];
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

	ID3D12Resource *i_resource = *depth_buffer.get();
	i_resource->AddRef();
	ID3D12Device *i_device = device_impl;
	ID3D12DescriptorHeap *i_heap = *heap_impl;
	size_t heap_slot = desc.resource_slot;
	std::unique_ptr<D3D12_CPU_DESCRIPTOR_HANDLE> dsv_handle = dx12_helpers_get_rw_descriptor_heap_handle(i_device, i_heap, heap_slot);
	i_device->CreateDepthStencilView(i_resource, &dsvDesc, *dsv_handle);
	return std::make_unique<DX_DEPTH_BUFFER>(i_resource, *dsv_handle,
											 desc.default_state, desc.format,
											 static_cast<size_t>(desc.width), static_cast<size_t>(desc.height));
}

std::unique_ptr<RHI_CONSTANT_BUFFER> dx12_buffers_create_constant(const RHI_BUFFER_DESC &desc)
{

	DX_DEVICE &device_impl = reinterpret_cast<DX_DEVICE &>(desc.device.get());
	DX_HEAP *heap_impl = device_impl.get_resources_heap();
	if (heap_impl == nullptr)
	{
		throw std::exception("NO heap found for dsv.");
	}
	auto buffer_impl = dx12_buffers_create_raw(desc);

	ID3D12Device *i_device = device_impl;
	ID3D12DescriptorHeap *i_heap = *heap_impl;

	// set heap offset
	size_t heap_slot = desc.resource_slot;
	// keep resource alive
	ID3D12Resource *i_resource = *buffer_impl;
	i_resource->AddRef();

	// create view
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbv_desc = {};
	cbv_desc.BufferLocation = i_resource->GetGPUVirtualAddress();
	cbv_desc.SizeInBytes = static_cast<UINT>(desc.length); // MUST BE ALIGNED
	std::unique_ptr<D3D12_CPU_DESCRIPTOR_HANDLE> cvb_handle = dx12_helpers_get_rw_descriptor_heap_handle(i_device, i_heap, heap_slot);
	i_device->CreateConstantBufferView(&cbv_desc, *cvb_handle);
	return std::make_unique<DX_CONSTANT_BUFFER>(i_resource, *cvb_handle,
												desc.default_state, desc.format,
												desc.length);
}

void dx12_buffers_copy_buffer(RHI_COMMAND_BUFFER &command_buffer, RHI_BUFFER &src_buffer, RHI_BUFFER &dest_buffer)
{
	static_cast<ID3D12GraphicsCommandList*>(command_buffer)->CopyResource(dest_buffer, src_buffer);
}

void dx12_buffers_copy_buffer_region(RHI_COMMAND_BUFFER &command_buffer, RHI_BUFFER &src_buffer,
									 size_t offset_src, RHI_BUFFER &dest_buffer,
									 size_t offset_dest, size_t length)
{
	static_cast<ID3D12GraphicsCommandList*>(command_buffer)->CopyBufferRegion(dest_buffer, offset_dest,
		src_buffer, offset_src, length);
}

void dx12_buffers_gpu_upload_region(RHI_COMMAND_BUFFER &command_buffer, RHI_BUFFER &cpu_buffer,
									RHI_BUFFER &gpu_buffer, size_t offset_src,
									size_t offset_dest, size_t length)
{

	ID3D12GraphicsCommandList* i_command_buffer = command_buffer;
	ID3D12Resource *i_dest_buffer = gpu_buffer;

	auto old_state = gpu_buffer.get_current_state();
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = i_dest_buffer;
	barrier.Transition.StateBefore = dx12_resource_state_type[old_state];
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	i_command_buffer->ResourceBarrier(1, &barrier);
	gpu_buffer.set_current_state(resource_state_copy_dest);
	dx12_buffers_copy_buffer_region(command_buffer, cpu_buffer,
									offset_src, gpu_buffer, offset_dest, length);
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = dx12_resource_state_type[old_state];
	i_command_buffer->ResourceBarrier(1, &barrier);
	gpu_buffer.set_current_state(old_state);
}

void dx12_buffers_gpu_upload(RHI_COMMAND_BUFFER &command_buffer, RHI_BUFFER &cpu_buffer,
							 RHI_BUFFER &gpu_buffer)
{

	ID3D12GraphicsCommandList* i_cmd_list = command_buffer;
	ID3D12Resource *i_dest_buffer = gpu_buffer;

	auto old_state = gpu_buffer.get_current_state();
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = i_dest_buffer;
	barrier.Transition.StateBefore = dx12_resource_state_type[old_state];
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	i_cmd_list->ResourceBarrier(1, &barrier);
	gpu_buffer.set_current_state(resource_state_copy_dest);
	dx12_buffers_copy_buffer(command_buffer, cpu_buffer,
							 gpu_buffer);
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = dx12_resource_state_type[old_state];
	i_cmd_list->ResourceBarrier(1, &barrier);
	gpu_buffer.set_current_state(old_state);
}

void dx12_buffers_gpu_download_region(RHI_COMMAND_BUFFER &command_buffer, RHI_BUFFER &cpu_buffer,
									  RHI_BUFFER &gpu_buffer, size_t offset_src,
									  size_t offset_dest, size_t length)
{

	ID3D12GraphicsCommandList* i_cmd_list = command_buffer;
	ID3D12Resource *i_src_buffer = gpu_buffer;

	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = i_src_buffer;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	i_cmd_list->ResourceBarrier(1, &barrier);
	dx12_buffers_copy_buffer_region(command_buffer, gpu_buffer,
									offset_src, cpu_buffer, offset_dest, length);
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
	i_cmd_list->ResourceBarrier(1, &barrier);
}

void dx12_buffers_gpu_download(RHI_COMMAND_BUFFER &command_buffer, RHI_BUFFER &cpu_buffer,
							   RHI_BUFFER &gpu_buffer)
{

	ID3D12GraphicsCommandList* i_cmd_list = command_buffer;
	ID3D12Resource *i_src_buffer = gpu_buffer;

	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = i_src_buffer;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	i_cmd_list->ResourceBarrier(1, &barrier);
	dx12_buffers_copy_buffer(command_buffer, gpu_buffer, cpu_buffer);
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
	i_cmd_list->ResourceBarrier(1, &barrier);
}

RHI_VOID_PTR dx12_buffers_map_open(RHI_BUFFER &cpu_buffer, size_t offset,
								   size_t length)
{

	ID3D12Resource *i_shared_buffer = cpu_buffer;
	D3D12_RANGE range{
		.Begin = offset,
		.End = length};

	RHI_VOID_PTR mapped;
	if (FAILED(i_shared_buffer->Map(0, &range, &mapped)))
	{
		throw std::exception("Error cannot map resource");
	}
	return mapped;
}

void dx12_buffers_map_close(RHI_BUFFER &cpu_buffer, size_t offset,
							size_t length)
{

	ID3D12Resource *i_shared_buffer = cpu_buffer;
	D3D12_RANGE range{
		.Begin = offset,
		.End = length};
	i_shared_buffer->Unmap(0, &range);
}

void dx12_buffers_map_write(RHI_BUFFER &shared_buffer,
							RHI_VOID_PTR data, size_t offset,
							size_t length)
{
	ID3D12Resource *i_shared_buffer = shared_buffer;
	D3D12_RANGE range{
		.Begin = offset,
		.End = length};
	void *mapped;
	if (FAILED(i_shared_buffer->Map(0, &range, &mapped)))
	{
		throw std::exception("Error cannot map resource");
	}
	memcpy(mapped, data, length);
	i_shared_buffer->Unmap(0, &range);
}

void dx12_buffers_map_read(RHI_BUFFER &shared_buffer, RHI_VOID_PTR &data,
						   size_t offset, size_t length)
{

	ID3D12Resource *i_shared_buffer = shared_buffer;
	D3D12_RANGE range{
		.Begin = offset,
		.End = length};
	void *mapped;
	if (FAILED(i_shared_buffer->Map(0, &range, &mapped)))
	{
		throw std::exception("Error cannot map resource");
	}
	memcpy(data, mapped, length);
	i_shared_buffer->Unmap(0, &range);
}