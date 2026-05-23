#include "dx12_buffers.hpp"
#include "dx12_command_buffer.hpp"

template <typename T>
T* dx12_buffers_create_2d(const RHI_BUFFER_2D_DESC* const desc)
{
	ASSERT_NULL(desc);

	D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;

	std::unique_ptr<D3D12_CLEAR_VALUE> clear_value;
	buffer_type buffer_type = desc->type;
	if (buffer_type == buffer_type_depth_stencil) {
		flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		clear_value = std::make_unique<D3D12_CLEAR_VALUE>();
		clear_value->Format = dx12_resource_format_type[desc->format];
		clear_value->DepthStencil.Depth = 1.0f;
		clear_value->DepthStencil.Stencil = 0;
	} else if (buffer_type == buffer_type_rt_bvh) {
		flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	} 
	if (desc->memory_type == buffer_memory_type_shared_rw) {
		flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	}
	if (desc->is_render_target == true) {
		flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	}
	
	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = dx12_heap_type[desc->memory_type];
	heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProps.CreationNodeMask = 1;
	heapProps.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC bufferDesc = {};
	bufferDesc.Dimension = dx12_buffer_type[buffer_type];
	bufferDesc.Alignment = 0;
	bufferDesc.Width = static_cast<UINT>(desc->width);
	bufferDesc.Height = static_cast<UINT>(desc->height);
	bufferDesc.DepthOrArraySize = 1;
	bufferDesc.MipLevels = static_cast<UINT>(desc->mips);
	bufferDesc.SampleDesc.Count = 1;
	bufferDesc.SampleDesc.Quality = 0;
	if (bufferDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
		bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	else
		bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	bufferDesc.Flags = flags;
	bufferDesc.Format = (bufferDesc.Dimension > D3D12_RESOURCE_DIMENSION_BUFFER)
		? dx12_resource_format_type[desc->format]
		: DXGI_FORMAT_UNKNOWN;
	ID3D12Resource* i_resource = nullptr;
	ID3D12Device* i_device = *static_cast<DX_DEVICE*>(desc->device);
	HRESULT hr = i_device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_COMMON,
		clear_value.get(),
		IID_PPV_ARGS(&i_resource));

	ASSERT_FAILED(hr);
	ASSERT_NULL(i_resource);

	T* buffer_impl = new T();
	ASSERT_NULL(buffer_impl);
	buffer_impl->length = desc->length;
	buffer_impl->format = desc->format;
	buffer_impl->current_state = D3D12_RESOURCE_STATE_COMMON;
	buffer_impl->set_handle(i_resource);

	return buffer_impl;
}

template <typename T>
T* dx12_buffers_create_raw(const RHI_BUFFER_DESC* const desc) {

	ASSERT_NULL(desc);

	RHI_BUFFER_2D_DESC desc_2d;
	desc_2d.length = desc->length;
	desc_2d.mips = desc->mips;
	desc_2d.memory_type = desc->memory_type;
	desc_2d.format = desc->format;
	desc_2d.type = desc->type;
	desc_2d.width = desc->length;
	desc_2d.height = 1;
	return dx12_buffers_create_2d<T>(&desc_2d);
}

RHI_DEPTH_BUFFER* dx12_buffers_create_depth(const RHI_DEPTH_BUFFER_DESC* const desc)
{
	ASSERT_NULL(desc);
	ASSERT_EXPR(desc->format >= resource_format_d32_float_s8_uint
		&& desc->format < resource_format_d16_norm);

	// overwrite desc to match must have depth buffer requeriments
	RHI_DEPTH_BUFFER_DESC db_desc_mutable = *desc;	
	db_desc_mutable.memory_type = buffer_memory_type_default;
	db_desc_mutable.type = buffer_type_depth_stencil;
	return dx12_buffers_create_2d<DX_DEPTH_BUFFER>(&db_desc_mutable);
}

RHI_CONSTANT_BUFFER* dx12_buffers_create_constant(const RHI_BUFFER_DESC* const desc)
{
	return dx12_buffers_create_raw<RHI_CONSTANT_BUFFER>(desc);
}

void dx12_buffers_copy_buffer(RHI_COMMAND_BUFFER* const command_buffer, const RHI_BUFFER* const src_buffer,
	RHI_BUFFER* const dest_buffer) {
	
	ASSERT_NULL(command_buffer);
	ASSERT_NULL(src_buffer);
	ASSERT_NULL(dest_buffer);

//	ID3D12GraphicsCommandList* i_command_buffer = static_cast<ID3D12GraphicsCommandList*>(*static_cast<DX_COMMAND_BUFFER*>(command_buffer));
//	ID3D12Resource* src = *static_cast<const DX_BUFFER*>(src_buffer);
//	ID3D12Resource* dest = *static_cast<DX_BUFFER*>(dest_buffer);
	static_cast<ID3D12GraphicsCommandList*>(
		*static_cast<DX_COMMAND_BUFFER*>(command_buffer))->CopyResource(
			*static_cast<DX_BUFFER*>(dest_buffer),
			*static_cast<const DX_BUFFER*>(src_buffer)
		);
}

void dx12_buffers_copy_buffer_region(RHI_COMMAND_BUFFER* const command_buffer, const RHI_BUFFER* const src_buffer,
	size_t offset_src, RHI_BUFFER* const dest_buffer,
	size_t offset_dest, size_t length)
{
	ASSERT_NULL(command_buffer);
	ASSERT_NULL(src_buffer);
	ASSERT_NULL(dest_buffer);

	//	ID3D12GraphicsCommandList* i_command_buffer = static_cast<ID3D12GraphicsCommandList*>(*static_cast<DX_COMMAND_BUFFER*>(command_buffer));
	//	ID3D12Resource* src = *static_cast<const DX_BUFFER*>(src_buffer);
	//	ID3D12Resource* dest = *static_cast<DX_BUFFER*>(dest_buffer);
	static_cast<ID3D12GraphicsCommandList*>(
		*static_cast<DX_COMMAND_BUFFER*>(command_buffer))->CopyBufferRegion(
			*static_cast<DX_BUFFER*>(dest_buffer), offset_dest, 
			*static_cast<const DX_BUFFER*>(src_buffer), offset_src, length
		);
}

void dx12_buffers_gpu_upload_region(RHI_COMMAND_BUFFER* const command_buffer, const RHI_BUFFER* const src_buffer,
	RHI_BUFFER* const dest_buffer, const size_t offset_src,
	const size_t offset_dest, const size_t length)
{
	ASSERT_NULL(command_buffer);
	ASSERT_NULL(src_buffer);
	ASSERT_NULL(dest_buffer);

	ID3D12GraphicsCommandList* i_command_buffer = static_cast<ID3D12GraphicsCommandList*>(*static_cast<DX_COMMAND_BUFFER*>(command_buffer));
	const DX_BUFFER* src = static_cast<const DX_BUFFER*>(src_buffer);
	DX_BUFFER* dest = static_cast<DX_BUFFER*>(dest_buffer);

	dx12_command_buffer_resource_transition_block(i_command_buffer, 
		dest,
		D3D12_RESOURCE_STATE_COPY_DEST,
		true,[&]() {
			dx12_buffers_copy_buffer_region(command_buffer, *src,
				offset_src, *dest, offset_dest, length);
		});
}

void dx12_buffers_gpu_upload(RHI_COMMAND_BUFFER* const command_buffer, const RHI_BUFFER* const src_buffer, 
	RHI_BUFFER* const dest_buffer)
{
	ASSERT_NULL(command_buffer);
	ASSERT_NULL(src_buffer);
	ASSERT_NULL(dest_buffer);

	ID3D12GraphicsCommandList* i_command_buffer = static_cast<ID3D12GraphicsCommandList*>(*static_cast<DX_COMMAND_BUFFER*>(command_buffer));
	const DX_BUFFER* src = static_cast<const DX_BUFFER*>(src_buffer);
	DX_BUFFER* dest = static_cast<DX_BUFFER*>(dest_buffer);

	dx12_command_buffer_resource_transition_block(i_command_buffer, 
		dest,
		D3D12_RESOURCE_STATE_COPY_DEST,
		true, [&]() {
			dx12_buffers_copy_buffer(command_buffer, *src,
				*dest);
		});
}

void dx12_buffers_gpu_download_region(RHI_COMMAND_BUFFER* const command_buffer, const RHI_BUFFER* const src_buffer,
	RHI_BUFFER* const dest_buffer, const size_t offset_src,
	const size_t offset_dest, const size_t length)
{

	ASSERT_NULL(command_buffer);
	ASSERT_NULL(src_buffer);
	ASSERT_NULL(dest_buffer);

	ID3D12GraphicsCommandList* i_command_buffer = static_cast<ID3D12GraphicsCommandList*>(*static_cast<DX_COMMAND_BUFFER*>(command_buffer));
	DX_BUFFER* src = const_cast<DX_BUFFER*>(static_cast<const DX_BUFFER*>(src_buffer));
	DX_BUFFER* dest = static_cast<DX_BUFFER*>(dest_buffer);
	dx12_command_buffer_resource_transition_block(i_command_buffer, 
		src,
		D3D12_RESOURCE_STATE_COPY_SOURCE,
		true, [&]() {
			dx12_buffers_copy_buffer_region(command_buffer, *src,
				offset_src, *dest, offset_dest, length);
		});
}

void dx12_buffers_gpu_download(RHI_COMMAND_BUFFER* const command_buffer, const RHI_BUFFER* const src_buffer,
	RHI_BUFFER* const dest_buffer)
{

	ASSERT_NULL(command_buffer);
	ASSERT_NULL(src_buffer);
	ASSERT_NULL(dest_buffer);

	ID3D12GraphicsCommandList* i_command_buffer = static_cast<ID3D12GraphicsCommandList*>(*static_cast<DX_COMMAND_BUFFER*>(command_buffer));
	DX_BUFFER* src = const_cast<DX_BUFFER*>(static_cast<const DX_BUFFER*>(src_buffer));
	DX_BUFFER* dest = static_cast<DX_BUFFER*>(dest_buffer);

	dx12_command_buffer_resource_transition_block(i_command_buffer, 
		src,
		D3D12_RESOURCE_STATE_COPY_SOURCE,
		true, [&]() {
			dx12_buffers_copy_buffer(command_buffer, *src, *dest);
		});
}

RHI_VOID_PTR dx12_buffers_map_open(RHI_BUFFER* const buffer, const size_t offset,
	const size_t length)
{
	ASSERT_NULL(buffer);

	ID3D12Resource* i_buffer = *static_cast<const DX_BUFFER*>(buffer);
	D3D12_RANGE range{
		.Begin = offset,
		.End = length};

	RHI_VOID_PTR mapped = nullptr;	
	ASSERT_FAILED(i_buffer->Map(0, &range, &mapped));
	ASSERT_NULL(mapped);
	
	return mapped;
}

void dx12_buffers_map_close(RHI_BUFFER* const buffer, const size_t offset,
	const size_t length)
{

	ASSERT_NULL(buffer);

	ID3D12Resource* i_buffer = *static_cast<const DX_BUFFER*>(buffer);
	D3D12_RANGE range{
		.Begin = offset,
		.End = length };

	RHI_VOID_PTR mapped = nullptr;
	i_buffer->Unmap(0, &range);
}

void dx12_buffers_map_write(RHI_BUFFER* const src_buffer, const RHI_VOID_PTR data,
	const size_t offset, const size_t length)
{
	RHI_VOID_PTR mapped = dx12_buffers_map_open(src_buffer, offset,
		length);
	memcpy(mapped, data, length);
	dx12_buffers_map_close(src_buffer, offset,
		length);
}

void dx12_buffers_map_read(RHI_BUFFER* const buffer, RHI_VOID_PTR* const data,
	const size_t offset, const size_t length)
{
	RHI_VOID_PTR mapped = dx12_buffers_map_open(buffer, offset,
		length);
	memcpy(data, mapped, length);
	dx12_buffers_map_close(buffer, offset,
		length);
}

RHI_VIEW* d12_buffers_create_dsv(const RHI_VIEW_DESC* const desc) {

	ASSERT_NULL(desc);
	ASSERT_NULL(desc->device);
	ASSERT_NULL(desc->buffer);

	ID3D12Device* i_device = *static_cast<DX_DEVICE*>(desc->device);
	ASSERT_NULL(i_device);

	ID3D12Resource* i_resource = *static_cast<DX_BUFFER*>(desc->buffer);
	ASSERT_NULL(i_resource);

	D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle = dx12_helpers_get_next_descriptor_heap_handle(desc->device, DSV_HEAP_ID);

	RHI_VIEW* result = nullptr;
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = dx12_resource_format_type[desc->format];
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	i_device->CreateDepthStencilView(i_resource, &dsvDesc, cpu_handle);

	ASSERT_NULL(result);
	return result;
}

RHI_VIEW* d12_buffers_create_rtv(const RHI_VIEW_DESC* const desc) {

	ASSERT_NULL(desc);
	ASSERT_NULL(desc->device);
	ASSERT_NULL(desc->buffer);

	ID3D12Device* i_device = *static_cast<DX_DEVICE*>(desc->device);
	ASSERT_NULL(i_device);

	ID3D12Resource* i_resource = *static_cast<DX_BUFFER*>(desc->buffer);
	ASSERT_NULL(i_resource);

	D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle = dx12_helpers_get_next_descriptor_heap_handle(desc->device, RTV_HEAP_ID);

	RHI_VIEW* result = nullptr;
	D3D12_RENDER_TARGET_VIEW_DESC rtv_desc = {};
	rtv_desc.Format = dx12_resource_format_type[desc->format];
	rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	i_device->CreateRenderTargetView(i_resource, &rtv_desc, cpu_handle);

	ASSERT_NULL(result);
	return result;
}

RHI_VIEW* d12_buffers_create_cbv_srv_uav(const RHI_VIEW_DESC* const desc) {

	ASSERT_NULL(desc);
	ASSERT_NULL(desc->device);
	ASSERT_NULL(desc->buffer);

	ID3D12Device* i_device = *static_cast<DX_DEVICE*>(desc->device);
	ASSERT_NULL(i_device);

	ID3D12Resource* i_resource = *static_cast<DX_BUFFER*>(desc->buffer);
	ASSERT_NULL(i_resource);

	D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle = dx12_helpers_get_next_descriptor_heap_handle(desc->device, RESOURCES_HEAP_ID);

	RHI_VIEW* result = nullptr;
	if (desc->type == resource_type_constant_buffer) {
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbv_desc = {};
		cbv_desc.BufferLocation = i_resource->GetGPUVirtualAddress();
		cbv_desc.SizeInBytes = static_cast<UINT>(desc->buffer->length); // MUST BE ALIGNED
		i_device->CreateConstantBufferView(&cbv_desc, cpu_handle);
	}
	else if (desc->type == resource_type_generic_rw_buffer) {
		D3D12_UNORDERED_ACCESS_VIEW_DESC uav_desc = {};
		uav_desc.Format = dx12_resource_format_type[desc->format];
		uav_desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		uav_desc.Buffer.FirstElement = 0;
		uav_desc.Buffer.NumElements = static_cast<UINT>(desc->buffer->length);
		uav_desc.Buffer.StructureByteStride = 0;
		uav_desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
		i_device->CreateUnorderedAccessView(i_resource, nullptr, &uav_desc, cpu_handle);
	}
	else if (desc->type == resource_type_shader) {
		D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
		srv_desc.Format = dx12_resource_format_type[desc->format];
		srv_desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srv_desc.Buffer.FirstElement = 0;
		srv_desc.Buffer.NumElements = static_cast<UINT>(desc->buffer->length);
		srv_desc.Buffer.StructureByteStride = 0;
		srv_desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		i_device->CreateShaderResourceView(i_resource, &srv_desc, cpu_handle);
	}
	result = new DX_VIEW();
	ASSERT_NULL(result);
	result->set_handle(&cpu_handle);
	return result;
}

RHI_VIEW* dx12_buffers_create_view(const RHI_VIEW_DESC* const desc) {

	ASSERT_NULL(desc);

	RHI_VIEW* result = nullptr;
	if (desc->type == resource_type_depth_stencil_target)
		result = d12_buffers_create_dsv(desc);
	else if (desc->type == resource_type_render_target)
		result = d12_buffers_create_rtv(desc);
	else
		result = d12_buffers_create_cbv_srv_uav(desc);

	ASSERT_NULL(result);
	return result;
}

RHI_INDEX_BUFFER* dx12_buffers_create_indices(const RHI_INDEX_BUFFER_DESC* const desc) {

	ASSERT_NULL(desc);

	RHI_BUFFER_DESC ib_desc;
	ib_desc.length = desc->count * desc->stride;
	ib_desc.memory_type = desc->memory_type;
	ib_desc.type = desc->type;
	ib_desc.format = desc->format;
	return dx12_buffers_create_raw<RHI_INDEX_BUFFER>(&ib_desc);
}

RHI_VERTEX_BUFFER* dx12_buffers_create_vertices(const RHI_VERTEX_BUFFER_DESC* const desc) {
	// overwrite desc to match must have index buffer requeriments
	RHI_BUFFER_DESC vb_desc;
	vb_desc.length = desc->count * desc->stride;
	vb_desc.memory_type = desc->memory_type;
	vb_desc.type = desc->type;
	vb_desc.format = desc->format;
	return dx12_buffers_create_raw<RHI_VERTEX_BUFFER>(&vb_desc);
}