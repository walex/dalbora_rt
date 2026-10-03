#include "dx12_buffers.hpp"
#include "dx12_command_buffer.hpp"
#include "dx12_heap.hpp"

template <typename T>
static T* buffers_create_2d_dx12(const RHI_BUFFER_2D_DESC* const desc)
{
	ASSERT_PTR(desc);
	ASSERT_PTR(desc->device);

	D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;
	D3D12_RESOURCE_STATES initial_state = D3D12_RESOURCE_STATE_COMMON;
	std::unique_ptr<D3D12_CLEAR_VALUE> clear_value;
	buffer_type buffer_type = desc->type;
	if (buffer_type == buffer_type_depth_stencil) {
		flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		clear_value = std::make_unique<D3D12_CLEAR_VALUE>();
		clear_value->Format = dx12_resource_format_type[desc->format];
		clear_value->DepthStencil.Depth = 1.0f;
		clear_value->DepthStencil.Stencil = 0;
		buffer_type = buffer_type_image_2d;
		initial_state = D3D12_RESOURCE_STATE_DEPTH_WRITE;
	}
	else if (buffer_type == buffer_type_bvh) {
		flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		initial_state = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
	}
	if ((desc->flags & resource_flags_shader_read_write
		) == resource_flags_shader_read_write) {
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
	bufferDesc.MipLevels = static_cast<UINT16>(desc->mips);
	bufferDesc.SampleDesc.Count = 1;
	bufferDesc.SampleDesc.Quality = 0;
	if (bufferDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
		bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	else
		bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	bufferDesc.Flags = flags;
	bufferDesc.Format = (bufferDesc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER)
		? dx12_resource_format_type[desc->format]
		: DXGI_FORMAT_UNKNOWN;
	ID3D12Resource* i_resource = nullptr;
	ID3D12Device* i_device = *static_cast<DX_DEVICE*>(desc->device);
	ASSERT_PTR(i_device);
	HRESULT hr = i_device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		initial_state,
		clear_value.get(),
		IID_PPV_ARGS(&i_resource));

	ASSERT_COM_SUCCESS(hr);
	ASSERT_PTR(i_resource);

	T* buffer_impl = new T();
	ASSERT_PTR(buffer_impl);
	buffer_impl->length = desc->length;
	buffer_impl->format = desc->format;
	buffer_impl->current_state = D3D12_RESOURCE_STATE_COMMON;
	buffer_impl->stride = desc->stride;
	buffer_impl->type = desc->type;
	buffer_impl->set_handle(i_resource);

	return buffer_impl;
}

template <typename T>
static T* buffers_create_dx12(const RHI_BUFFER_DESC* const desc) {

	ASSERT_PTR(desc);
	ASSERT_PTR(desc->device);

	RHI_BUFFER_2D_DESC desc_2d;
	desc_2d.device = desc->device;
	desc_2d.length = desc->length;
	desc_2d.mips = desc->mips;
	desc_2d.memory_type = desc->memory_type;
	desc_2d.format = desc->format;
	desc_2d.type = desc->type;
	desc_2d.width = desc->length;
	desc_2d.height = 1;
	desc_2d.stride = desc->stride;
	return buffers_create_2d_dx12<T>(&desc_2d);
}

static void buffers_create_dsv_from_handle_dx12(ID3D12Device* const i_device,
	ID3D12Resource* const i_resource,
	resource_format format,
	D3D12_CPU_DESCRIPTOR_HANDLE handle) {

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = dx12_resource_format_type[format];
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	i_device->CreateDepthStencilView(i_resource, &dsvDesc, handle);

}

static RHI_VIEW* buffers_create_dsv_dx12(const RHI_VIEW_DESC* const desc) {

	ASSERT_PTR(desc);
	ASSERT_PTR(desc->device);
	ASSERT_PTR(desc->buffer);
	ASSERT_PTR(desc->memory_descriptor);

	ID3D12Device* i_device = *static_cast<DX_DEVICE*>(desc->device);
	ASSERT_PTR(i_device);

	ID3D12Resource* i_resource = *static_cast<DX_BUFFER*>(desc->buffer);
	ASSERT_PTR(i_resource);

	DX_VIEW* result = new DX_VIEW();
	ASSERT_PTR(result);

	buffers_create_dsv_from_handle_dx12(i_device, i_resource, desc->format,
		{ static_cast<const DX_MEMORY_DESCRIPTOR_SLOT*>(desc->memory_descriptor)->cpu_handle });
	result->buffer = desc->buffer;
	result->type = desc->type;
	result->format = desc->format;
	result->mip_map_count = 1;
	result->memory_descriptor = desc->memory_descriptor;
	return result;
}

static void buffers_create_rtv_from_handle_dx12(ID3D12Device* const i_device,
	ID3D12Resource* const i_resource,
	resource_format format,
	D3D12_CPU_DESCRIPTOR_HANDLE handle) {

	D3D12_RENDER_TARGET_VIEW_DESC rtv_desc = {};
	rtv_desc.Format = dx12_resource_format_type[format];
	rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	i_device->CreateRenderTargetView(i_resource, &rtv_desc, handle);
}

static RHI_VIEW* buffers_create_rtv_dx12(const RHI_VIEW_DESC* const desc) {

	ASSERT_PTR(desc);
	ASSERT_PTR(desc->device);
	ASSERT_PTR(desc->buffer);
	ASSERT_PTR(desc->memory_descriptor);

	ID3D12Device* i_device = *static_cast<DX_DEVICE*>(desc->device);
	ASSERT_PTR(i_device);

	ID3D12Resource* i_resource = *static_cast<DX_BUFFER*>(desc->buffer);
	ASSERT_PTR(i_resource);

	DX_VIEW* result = new DX_VIEW();
	ASSERT_PTR(result);

	buffers_create_rtv_from_handle_dx12(i_device, i_resource,
		desc->format, { static_cast<const DX_MEMORY_DESCRIPTOR_SLOT*>(desc->memory_descriptor)->cpu_handle });
	result->buffer = desc->buffer;
	result->type = desc->type;
	result->format = desc->format;
	result->mip_map_count = 1;
	result->memory_descriptor = desc->memory_descriptor;
	return result;
}

static void buffers_create_cbv_srv_uav_from_handle_dx12(ID3D12Device* const i_device,
	ID3D12Resource* const i_resource,
	shader_view_type type,
	size_t buffer_length,
	size_t buffer_stride,
	resource_format format,
	size_t mip_maps_count,
	D3D12_CPU_DESCRIPTOR_HANDLE handle) {

	if (type == shader_view_type_constant_buffer) {
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbv_desc = {};
		cbv_desc.BufferLocation = i_resource->GetGPUVirtualAddress();
		cbv_desc.SizeInBytes = static_cast<UINT>(buffer_length); // MUST BE ALIGNED
		i_device->CreateConstantBufferView(&cbv_desc, handle);
	}
	else if (type == shader_view_type_rw_buffer) {
		D3D12_UNORDERED_ACCESS_VIEW_DESC uav_desc = {};
		uav_desc.Format = dx12_resource_format_type[format];
		uav_desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		uav_desc.Buffer.FirstElement = 0;
		uav_desc.Buffer.NumElements = static_cast<UINT>(buffer_length / buffer_stride);
		uav_desc.Buffer.StructureByteStride = 0;
		uav_desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
		i_device->CreateUnorderedAccessView(i_resource, nullptr, &uav_desc, handle);
	}
	else if (type == shader_view_type_read_only_buffer) {
		D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
		srv_desc.Format = dx12_resource_format_type[format];
		srv_desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srv_desc.Shader4ComponentMapping =
			D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srv_desc.Buffer.FirstElement = 0;
		srv_desc.Buffer.NumElements = static_cast<UINT>(buffer_length / buffer_stride);
		srv_desc.Buffer.StructureByteStride = 0;
		srv_desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		i_device->CreateShaderResourceView(i_resource, &srv_desc, handle);
	}
	else if (type == shader_view_type_rw_texture_buffer) {
		D3D12_UNORDERED_ACCESS_VIEW_DESC uav_desc = {};
		uav_desc.Format = dx12_resource_format_type[format];
		uav_desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		//	uav_desc.Buffer.FirstElement = 0;
		//	uav_desc.Buffer.NumElements = static_cast<UINT>(buffer_length);
		//	uav_desc.Buffer.StructureByteStride = 0;
		//	uav_desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
		i_device->CreateUnorderedAccessView(i_resource, nullptr, &uav_desc, handle);
	}
	else if (type == shader_view_type_read_only_texture_buffer) {
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
	else if (type == shader_view_type_bvh_buffer) {
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

static RHI_VIEW* buffers_create_cbv_srv_uav_dx12(const RHI_VIEW_DESC* const desc) {

	ASSERT_PTR(desc);
	ASSERT_PTR(desc->device);
	ASSERT_PTR(desc->buffer);

	ID3D12Device* i_device = *static_cast<DX_DEVICE*>(desc->device);
	ASSERT_PTR(i_device);

	ID3D12Resource* i_resource = *static_cast<DX_BUFFER*>(desc->buffer);
	ASSERT_PTR(i_resource);


	buffers_create_cbv_srv_uav_from_handle_dx12(i_device,
		i_resource,
		desc->type,
		desc->buffer->length,
		desc->buffer->stride,
		desc->format,
		desc->mip_maps_count,
		{ static_cast<const DX_MEMORY_DESCRIPTOR_SLOT*>(desc->memory_descriptor)->cpu_handle });

	DX_VIEW* result = new DX_VIEW();
	ASSERT_PTR(result);
	result->buffer = desc->buffer;
	result->type = desc->type;
	result->format = desc->format;
	result->mip_map_count = desc->mip_maps_count;
	result->memory_descriptor = desc->memory_descriptor;
	return result;
}

RHI_BUFFER* dx12_buffers_create_linear(const RHI_BUFFER_DESC* const desc) {
	return buffers_create_dx12<DX_BUFFER>(desc);
}

RHI_BUFFER* dx12_buffers_create_2d(const RHI_BUFFER_2D_DESC* const desc) {
	return buffers_create_2d_dx12<DX_BUFFER>(desc);
}

RHI_BUFFER* dx12_buffers_create_constant(const RHI_BUFFER_DESC* const desc)
{
	return buffers_create_dx12<DX_BUFFER>(desc);
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
	return buffers_create_dx12<DX_BUFFER>(&ib_desc);
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
	return buffers_create_dx12<DX_BUFFER>(&vb_desc);
}

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
	return buffers_create_2d_dx12<DX_TEXTURE_2D>(&db_desc_mutable);
}

void dx12_buffers_copy_buffer_region(RHI_COMMAND_BUFFER* const command_buffer, const RHI_BUFFER* const src_buffer,
	size_t offset_src, RHI_BUFFER* const dest_buffer,
	size_t offset_dest, size_t length)
{
	ASSERT_PTR(command_buffer);
	ASSERT_PTR(src_buffer);
	ASSERT_PTR(dest_buffer);

	static_cast<ID3D12GraphicsCommandList*>(
		*static_cast<DX_COMMAND_BUFFER*>(command_buffer))->CopyBufferRegion(
			*static_cast<DX_BUFFER*>(dest_buffer), offset_dest,
			*static_cast<const DX_BUFFER*>(src_buffer), offset_src, length
		);
}

void dx12_buffers_copy_buffer(RHI_COMMAND_BUFFER* const command_buffer, const RHI_BUFFER* const src_buffer,
	RHI_BUFFER* const dest_buffer) {

	ASSERT_PTR(command_buffer);
	ASSERT_PTR(src_buffer);
	ASSERT_PTR(dest_buffer);

	static_cast<ID3D12GraphicsCommandList*>(
		*static_cast<DX_COMMAND_BUFFER*>(command_buffer))->CopyResource(
			*static_cast<DX_BUFFER*>(dest_buffer),
			*static_cast<const DX_BUFFER*>(src_buffer)
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
	DX_BUFFER* dest = static_cast<DX_BUFFER*>(dest_buffer);

	dx12_command_buffer_resource_barrier_transition_and_restore(i_command_buffer,
		{ dest },
		{ D3D12_RESOURCE_STATE_COPY_DEST },
		[&]() {
			dx12_buffers_copy_buffer_region(command_buffer, src_buffer,
				offset_src, dest_buffer, offset_dest, length);
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

	dx12_command_buffer_resource_barrier_transition_and_restore(i_command_buffer,
		{ dest },
		{ D3D12_RESOURCE_STATE_COPY_DEST },
		[&]() {
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

	dx12_command_buffer_resource_barrier_transition_and_restore(i_command_buffer,
		{ src },
		{ D3D12_RESOURCE_STATE_COPY_SOURCE },
		[&]() {
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

	dx12_command_buffer_resource_barrier_transition_and_restore(i_command_buffer,
		{ src },
		{ D3D12_RESOURCE_STATE_COPY_SOURCE },
		[&]() {
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
		.End = offset+length};

	RHI_VOID_PTR mapped = nullptr;	
	ASSERT_COM_SUCCESS(i_buffer->Map(0, &range, &mapped));
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

RHI_VIEW* dx12_buffers_create_view(const RHI_VIEW_DESC* const desc) {

	ASSERT_PTR(desc);

	RHI_VIEW* result = nullptr;
	if (desc->type == shader_view_type_depth_stencil_target)
		result = buffers_create_dsv_dx12(desc);
	else if (desc->type == shader_view_type_render_target)
		result = buffers_create_rtv_dx12(desc);
	else
		result = buffers_create_cbv_srv_uav_dx12(desc);

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
	D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle{ static_cast<const DX_MEMORY_DESCRIPTOR_SLOT*>(view_impl->memory_descriptor)->cpu_handle };
	if (view_impl->type == shader_view_type_depth_stencil_target)
		buffers_create_dsv_from_handle_dx12(i_device,
			i_resource,			
			view_impl->format,
			cpu_handle);
	else if (view->type == shader_view_type_render_target)
		 buffers_create_rtv_from_handle_dx12(i_device,
			i_resource,
			view_impl->format,
			cpu_handle);
	else
		buffers_create_cbv_srv_uav_from_handle_dx12(i_device,
			i_resource,
			view_impl->type,
			view_impl->buffer->length,
			view_impl->buffer->stride,
			view_impl->format,
			view_impl->mip_map_count,
			cpu_handle);
}