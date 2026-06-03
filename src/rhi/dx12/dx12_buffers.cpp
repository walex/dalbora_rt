#include "dx12_buffers.hpp"
#include "dx12_command_buffer.hpp"
#include "dx12_heap.hpp"

RHI_BUFFER* dx12_buffers_create_depth(const RHI_BUFFER_2D_DESC* const desc)
{
	ASSERT_PTR(desc);
	ASSERT_EXPR(desc->format >= resource_format_d32_float_s8_uint
		&& desc->format < resource_format_d16_norm);

	// overwrite desc to match must have depth buffer requeriments
	RHI_BUFFER_2D_DESC db_desc_mutable = *desc;
	db_desc_mutable.memory_type = buffer_memory_type_default;
	db_desc_mutable.type = buffer_type_depth_stencil;
	db_desc_mutable.mips = 1;
	db_desc_mutable.width = desc->width;
	db_desc_mutable.height = desc->height;
	db_desc_mutable.format = desc->format;
	return dx12_buffers_create_2d<DX_TEXTURE_2D>(&db_desc_mutable);
}

RHI_BUFFER* dx12_buffers_create_constant(const RHI_BUFFER_DESC* const desc)
{
	return dx12_buffers_create<DX_BUFFER>(desc);
}

void dx12_buffers_copy_buffer(RHI_COMMAND_BUFFER* const command_buffer, const RHI_BUFFER* const src_buffer,
	RHI_BUFFER* const dest_buffer) {
	
	ASSERT_PTR(command_buffer);
	ASSERT_PTR(src_buffer);
	ASSERT_PTR(dest_buffer);

	// ID3D12GraphicsCommandList* i_command_buffer = static_cast<ID3D12GraphicsCommandList*>(*static_cast<DX_COMMAND_BUFFER*>(command_buffer));
	// ID3D12Resource* src = *static_cast<const DX_BUFFER*>(src_buffer);
	// ID3D12Resource* dest = *static_cast<DX_BUFFER*>(dest_buffer);
	// i_command_buffer->CopyResource(dest, src);
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
	ASSERT_PTR(command_buffer);
	ASSERT_PTR(src_buffer);
	ASSERT_PTR(dest_buffer);

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
	ASSERT_PTR(command_buffer);
	ASSERT_PTR(src_buffer);
	ASSERT_PTR(dest_buffer);

	ID3D12GraphicsCommandList* i_command_buffer = static_cast<ID3D12GraphicsCommandList*>(*static_cast<DX_COMMAND_BUFFER*>(command_buffer));
	const DX_BUFFER* src = static_cast<const DX_BUFFER*>(src_buffer);
	DX_BUFFER* dest = static_cast<DX_BUFFER*>(dest_buffer);
	
	static constexpr D3D12_RESOURCE_STATES resource_state[] = { D3D12_RESOURCE_STATE_COPY_DEST };
	static constexpr bool restore[] = { true };
	DX_RESOURCE* resources[] = { dest };
	dx12_command_buffer_resource_transition(i_command_buffer, 
		resources,
		resource_state,
		restore,1, [&]() {
			dx12_buffers_copy_buffer_region(command_buffer, *src,
				offset_src, *dest, offset_dest, length);
		});
}

void dx12_buffers_gpu_upload(RHI_COMMAND_BUFFER* const command_buffer, const RHI_BUFFER* const src_buffer, 
	RHI_BUFFER* const dest_buffer)
{
	ASSERT_PTR(command_buffer);
	ASSERT_PTR(src_buffer);
	ASSERT_PTR(dest_buffer);

	ID3D12GraphicsCommandList* i_command_buffer = static_cast<ID3D12GraphicsCommandList*>(*static_cast<DX_COMMAND_BUFFER*>(command_buffer));
	ASSERT_PTR(i_command_buffer);
	const DX_BUFFER* src = static_cast<const DX_BUFFER*>(src_buffer);
	DX_BUFFER* dest = static_cast<DX_BUFFER*>(dest_buffer);

	static constexpr D3D12_RESOURCE_STATES resource_state[] = {D3D12_RESOURCE_STATE_COPY_DEST};
	static constexpr bool restore[] = { true };
	DX_RESOURCE* resources[] = { dest };
	dx12_command_buffer_resource_transition(i_command_buffer,
		resources,
		resource_state,
		restore, 1, [&]() {
			dx12_buffers_copy_buffer(command_buffer, src,
				dest);
		});
}

void dx12_buffers_gpu_download_region(RHI_COMMAND_BUFFER* const command_buffer, const RHI_BUFFER* const src_buffer,
	RHI_BUFFER* const dest_buffer, const size_t offset_src,
	const size_t offset_dest, const size_t length)
{

	ASSERT_PTR(command_buffer);
	ASSERT_PTR(src_buffer);
	ASSERT_PTR(dest_buffer);

	ID3D12GraphicsCommandList* i_command_buffer = static_cast<ID3D12GraphicsCommandList*>(*static_cast<DX_COMMAND_BUFFER*>(command_buffer));
	DX_BUFFER* src = const_cast<DX_BUFFER*>(static_cast<const DX_BUFFER*>(src_buffer));
	DX_BUFFER* dest = static_cast<DX_BUFFER*>(dest_buffer);

	static constexpr D3D12_RESOURCE_STATES resource_state[] = {D3D12_RESOURCE_STATE_COPY_SOURCE};
	static constexpr bool restore[] = {true};
	DX_RESOURCE* resources[] = { src };
	dx12_command_buffer_resource_transition(i_command_buffer,
		resources,
		resource_state,
		restore, 1, [&]() {
			dx12_buffers_copy_buffer_region(command_buffer, *src,
				offset_src, *dest, offset_dest, length);
		});
}

void dx12_buffers_gpu_download(RHI_COMMAND_BUFFER* const command_buffer, const RHI_BUFFER* const src_buffer,
	RHI_BUFFER* const dest_buffer)
{

	ASSERT_PTR(command_buffer);
	ASSERT_PTR(src_buffer);
	ASSERT_PTR(dest_buffer);

	ID3D12GraphicsCommandList* i_command_buffer = static_cast<ID3D12GraphicsCommandList*>(*static_cast<DX_COMMAND_BUFFER*>(command_buffer));
	DX_BUFFER* src = const_cast<DX_BUFFER*>(static_cast<const DX_BUFFER*>(src_buffer));
	DX_BUFFER* dest = static_cast<DX_BUFFER*>(dest_buffer);

	static constexpr D3D12_RESOURCE_STATES resource_state[] = { D3D12_RESOURCE_STATE_COPY_SOURCE };
	static constexpr bool restore[] = { true };
	DX_RESOURCE* resources[] = { src };
	dx12_command_buffer_resource_transition(i_command_buffer,
		resources,
		resource_state,
		restore, 1, [&]() {
			dx12_buffers_copy_buffer(command_buffer, *src, *dest);
		});
}

RHI_VOID_PTR dx12_buffers_map_open(RHI_BUFFER* const buffer, const size_t offset,
	const size_t length)
{
	ASSERT_PTR(buffer);

	ID3D12Resource* i_buffer = *static_cast<const DX_BUFFER*>(buffer);
	D3D12_RANGE range{
		.Begin = offset,
		.End = length};

	RHI_VOID_PTR mapped = nullptr;	
	ASSERT_SUCCESS(i_buffer->Map(0, &range, &mapped));
	ASSERT_PTR(mapped);
	
	return mapped;
}

void dx12_buffers_map_close(RHI_BUFFER* const buffer, const size_t offset,
	const size_t length)
{

	ASSERT_PTR(buffer);

	ID3D12Resource* i_buffer = *static_cast<const DX_BUFFER*>(buffer);
	D3D12_RANGE range{
		.Begin = offset,
		.End = length };
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

void dx12_buffers_create_dsv_from_handle(ID3D12Device* const i_device,
	ID3D12Resource* const i_resource,
	resource_format format,
	D3D12_CPU_DESCRIPTOR_HANDLE handle) {

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = dx12_resource_format_type[format];
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	i_device->CreateDepthStencilView(i_resource, &dsvDesc, handle);

}

RHI_VIEW* dx12_buffers_create_dsv(const RHI_VIEW_DESC* const desc) {

	ASSERT_PTR(desc);
	ASSERT_PTR(desc->device);
	ASSERT_PTR(desc->buffer);

	ID3D12Device* i_device = *static_cast<DX_DEVICE*>(desc->device);
	ASSERT_PTR(i_device);

	ID3D12Resource* i_resource = *static_cast<DX_BUFFER*>(desc->buffer);
	ASSERT_PTR(i_resource);

	DX_VIEW* result = new DX_VIEW();
	ASSERT_PTR(result);

	result->descriptor_size = dx12_heap_next_handle(
		static_cast<DX_DEVICE*>(desc->device), 
		heap_id_type_dsv, 
		desc->slot_id,
		&result->cpu_descriptor_handle, 
		&result->gpu_descriptor_handle);

	dx12_buffers_create_dsv_from_handle(i_device, i_resource, desc->format, result->cpu_descriptor_handle);
	result->buffer = desc->buffer;
	result->type = desc->type;
	result->format = desc->format;
	result->mip_map_count = 1;
	return result;
}

void dx12_buffers_create_rtv_from_handle(ID3D12Device* const i_device,
	ID3D12Resource* const i_resource,
	resource_format format,
	D3D12_CPU_DESCRIPTOR_HANDLE handle) {

	D3D12_RENDER_TARGET_VIEW_DESC rtv_desc = {};
	rtv_desc.Format = dx12_resource_format_type[format];
	rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	i_device->CreateRenderTargetView(i_resource, &rtv_desc, handle);
}

RHI_VIEW* dx12_buffers_create_rtv(const RHI_VIEW_DESC* const desc) {

	ASSERT_PTR(desc);
	ASSERT_PTR(desc->device);
	ASSERT_PTR(desc->buffer);

	ID3D12Device* i_device = *static_cast<DX_DEVICE*>(desc->device);
	ASSERT_PTR(i_device);

	ID3D12Resource* i_resource = *static_cast<DX_BUFFER*>(desc->buffer);
	ASSERT_PTR(i_resource);

	DX_VIEW* result = new DX_VIEW();
	ASSERT_PTR(result);

	dx12_heap_next_handle(
		static_cast<DX_DEVICE*>(desc->device), 
		heap_id_type_rtv, 
		desc->slot_id,
		&result->cpu_descriptor_handle,
		&result->gpu_descriptor_handle);

	dx12_buffers_create_rtv_from_handle(i_device, i_resource,
		desc->format, result->cpu_descriptor_handle);
	result->buffer = desc->buffer;
	result->type = desc->type;
	result->format = desc->format;
	result->mip_map_count = 1;
	return result;
}

void dx12_buffers_create_cbv_srv_uav_from_handle(ID3D12Device* const i_device, 
	ID3D12Resource* const i_resource,
	resource_type type,
	size_t buffer_length,
	resource_format format,
	size_t mip_maps_count,
	D3D12_CPU_DESCRIPTOR_HANDLE handle) {

	if (type == resource_type_constant_buffer) {
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbv_desc = {};
		cbv_desc.BufferLocation = i_resource->GetGPUVirtualAddress();
		cbv_desc.SizeInBytes = static_cast<UINT>(buffer_length); // MUST BE ALIGNED
		i_device->CreateConstantBufferView(&cbv_desc, handle);
	}
	else if (type == resource_type_generic_rw_buffer) {
		D3D12_UNORDERED_ACCESS_VIEW_DESC uav_desc = {};
		uav_desc.Format = dx12_resource_format_type[format];
		uav_desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		uav_desc.Buffer.FirstElement = 0;
		uav_desc.Buffer.NumElements = static_cast<UINT>(buffer_length);
		uav_desc.Buffer.StructureByteStride = 0;
		uav_desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
		i_device->CreateUnorderedAccessView(i_resource, nullptr, &uav_desc, handle);
	}
	else if (type == resource_type_shader) {
		D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
		srv_desc.Format = dx12_resource_format_type[format];
		srv_desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srv_desc.Buffer.FirstElement = 0;
		srv_desc.Buffer.NumElements = static_cast<UINT>(buffer_length);
		srv_desc.Buffer.StructureByteStride = 0;
		srv_desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		i_device->CreateShaderResourceView(i_resource, &srv_desc, handle);
	}
	else if (type == resource_type_texture_2d_rw) {
		D3D12_UNORDERED_ACCESS_VIEW_DESC uav_desc = {};
		uav_desc.Format = dx12_resource_format_type[format];
		uav_desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	//	uav_desc.Buffer.FirstElement = 0;
	//	uav_desc.Buffer.NumElements = static_cast<UINT>(buffer_length);
	//	uav_desc.Buffer.StructureByteStride = 0;
	//	uav_desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
		i_device->CreateUnorderedAccessView(i_resource, nullptr, &uav_desc, handle);
	}
	else if (type == resource_type_texture_2d_read_only) {
		D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
		srv_desc.Format = dx12_resource_format_type[format];
		srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srv_desc.Shader4ComponentMapping =
			D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srv_desc.Texture2D.MipLevels = static_cast<UINT>(mip_maps_count);
		srv_desc.Texture1D.MostDetailedMip = 0;
		i_device->CreateShaderResourceView(
			i_resource,
			&srv_desc,
			handle);
	}
	else if (type == resource_type_rt_bvh_buffer) {
		D3D12_SHADER_RESOURCE_VIEW_DESC srv = {};
		srv.ViewDimension =
			D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
		srv.Shader4ComponentMapping =
			D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srv.RaytracingAccelerationStructure.Location =
			i_resource->GetGPUVirtualAddress();
		i_device->CreateShaderResourceView(
			nullptr,
			&srv,
			handle
		);
	}
}

RHI_VIEW* dx12_buffers_create_cbv_srv_uav(const RHI_VIEW_DESC* const desc) {

	ASSERT_PTR(desc);
	ASSERT_PTR(desc->device);
	ASSERT_PTR(desc->buffer);

	ID3D12Device* i_device = *static_cast<DX_DEVICE*>(desc->device);
	ASSERT_PTR(i_device);

	ID3D12Resource* i_resource = *static_cast<DX_BUFFER*>(desc->buffer);
	ASSERT_PTR(i_resource);

	D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle;
	D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle;
	size_t descriptor_size = 
		dx12_heap_next_handle(
			static_cast<DX_DEVICE*>(desc->device), 
			heap_id_type_resources, 
			desc->slot_id,
			&cpu_handle, 
			&gpu_handle);

	dx12_buffers_create_cbv_srv_uav_from_handle(i_device,
		i_resource,
		desc->type,
		desc->buffer->length,
		desc->format,
		desc->mip_maps_count,
		cpu_handle);

	DX_VIEW* result = new DX_VIEW();
	ASSERT_PTR(result);
	result->cpu_descriptor_handle = cpu_handle;
	result->gpu_descriptor_handle = gpu_handle;
	result->descriptor_size = descriptor_size;
	result->buffer = desc->buffer;
	result->type = desc->type;
	result->format = desc->format;
	result->mip_map_count = desc->mip_maps_count;
	return result;
}

RHI_VIEW* dx12_buffers_create_view(const RHI_VIEW_DESC* const desc) {

	ASSERT_PTR(desc);

	RHI_VIEW* result = nullptr;
	if (desc->type == resource_type_depth_stencil_target)
		result = dx12_buffers_create_dsv(desc);
	else if (desc->type == resource_type_render_target)
		result = dx12_buffers_create_rtv(desc);
	else
		result = dx12_buffers_create_cbv_srv_uav(desc);

	ASSERT_PTR(result);
	return result;
}

void dx12_buffers_update_view(const RHI_DEVICE* const device, 
	RHI_VIEW* const view, 
	const RHI_BUFFER* const buffer) {

	ASSERT_PTR(device);
	ASSERT_PTR(view);
	ASSERT_PTR(buffer);

	ID3D12Device* i_device = *static_cast<const DX_DEVICE*>(device);
	ASSERT_PTR(i_device);

	ID3D12Resource* i_resource = *static_cast<const DX_BUFFER*>(buffer);
	ASSERT_PTR(i_resource);

	DX_VIEW* view_impl = static_cast<DX_VIEW*>(view);

	if (view_impl->type == resource_type_depth_stencil_target)
		dx12_buffers_create_dsv_from_handle(i_device,
			i_resource,			
			view_impl->format,
			view_impl->cpu_descriptor_handle);
	else if (view->type == resource_type_render_target)
		 dx12_buffers_create_rtv_from_handle(i_device,
			i_resource,
			view_impl->format,
			view_impl->cpu_descriptor_handle);
	else
		dx12_buffers_create_cbv_srv_uav_from_handle(i_device,
			i_resource,
			view_impl->type,
			view_impl->buffer->length,
			view_impl->format,
			view_impl->mip_map_count,
			view_impl->cpu_descriptor_handle);
}

RHI_BUFFER* dx12_buffers_create_indices(const RHI_INDEX_BUFFER_DESC* const desc) {

	ASSERT_PTR(desc);
	ASSERT_PTR(desc->device);

	RHI_BUFFER_DESC ib_desc;
	ib_desc.device = desc->device;
	ib_desc.length = desc->count * desc->stride;
	ib_desc.memory_type = desc->memory_type;
	ib_desc.type = desc->type;
	ib_desc.format = desc->format;
	ib_desc.mips = 1;
	ib_desc.stride = desc->stride;
	return dx12_buffers_create<DX_BUFFER>(&ib_desc);
}

RHI_BUFFER* dx12_buffers_create_vertices(const RHI_VERTEX_BUFFER_DESC* const desc) {
	
	ASSERT_PTR(desc);
	ASSERT_PTR(desc->device);

	// overwrite desc to match must have index buffer requeriments
	RHI_BUFFER_DESC vb_desc;
	vb_desc.device = desc->device;
	vb_desc.length = desc->count * desc->stride;
	vb_desc.memory_type = desc->memory_type;
	vb_desc.type = desc->type;
	vb_desc.format = desc->format;
	vb_desc.mips = 1;
	vb_desc.stride = desc->stride;
	return dx12_buffers_create<DX_BUFFER>(&vb_desc);
}