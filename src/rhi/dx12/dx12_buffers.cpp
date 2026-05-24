#include "dx12_buffers.hpp"
#include "dx12_command_buffer.hpp"
#include "dx12_heap.hpp"

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

	D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle;
	dx12_heap_next_handle(static_cast<DX_DEVICE*>(desc->device), heap_id_type_dsv, &cpu_handle);

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

	D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle;
	dx12_heap_next_handle(static_cast<DX_DEVICE*>(desc->device), heap_id_type_rtv, &cpu_handle);

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

	D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle;
	D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle;
	size_t descriptor_size = dx12_heap_next_handle(static_cast<DX_DEVICE*>(desc->device), heap_id_type_resources, &cpu_handle, &gpu_handle);

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
	DX_VIEW* result = new DX_VIEW();
	ASSERT_NULL(result);
	result->cpu_descriptor_handle = cpu_handle;
	result->gpu_descriptor_handle = gpu_handle;
	result->descriptor_size = descriptor_size;
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