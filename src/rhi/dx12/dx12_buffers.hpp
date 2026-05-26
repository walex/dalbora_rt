#ifndef __dx12_buffers_hpp__
#define __dx12_buffers_hpp__

#include "dx12_rhi.hpp"

RHI_BUFFER* dx12_buffers_create_depth(const RHI_BUFFER_2D_DESC* const desc);
RHI_BUFFER* dx12_buffers_create_constant(const RHI_BUFFER_DESC* const desc);
void dx12_buffers_copy_buffer_region(RHI_COMMAND_BUFFER* const command_buffer, const RHI_BUFFER* const src_buffer,
	size_t offset_src, RHI_BUFFER* const dest_buffer, 
	size_t offset_dest, size_t length);
void dx12_buffers_gpu_upload_region(RHI_COMMAND_BUFFER* const command_buffer, const RHI_BUFFER* const src_buffer,
	RHI_BUFFER* const dest_buffer, const size_t offset_src, 
	const size_t offset_dest, const size_t length);
void dx12_buffers_gpu_upload(RHI_COMMAND_BUFFER* const command_buffer, const RHI_BUFFER* const src_buffer, 
	RHI_BUFFER* const gpu_buffer);
void dx12_buffers_gpu_download_region(RHI_COMMAND_BUFFER* const command_buffer, const RHI_BUFFER* const src_buffer,
	RHI_BUFFER* const gpu_buffer, const size_t offset_src, 
	const size_t offset_dest, const size_t length);
void dx12_buffers_gpu_download(RHI_COMMAND_BUFFER* const command_buffer, const RHI_BUFFER* const src_buffer, 
	RHI_BUFFER* const gpu_buffer);
RHI_VOID_PTR dx12_buffers_map_open(RHI_BUFFER* const buffer, const size_t offset,
	const size_t length);
void dx12_buffers_map_close(RHI_BUFFER* const buffer, const size_t offset,
	const size_t length);
void dx12_buffers_copy_buffer(RHI_COMMAND_BUFFER* const command_buffer, const RHI_BUFFER* const src_buffer,
	RHI_BUFFER* const dest_buffer);
void dx12_buffers_map_write(RHI_BUFFER* const buffer, const RHI_VOID_PTR data,
	const size_t offset, const size_t length);
void dx12_buffers_map_read(RHI_BUFFER* const buffer, RHI_VOID_PTR* const data,
	const size_t offset, const size_t length);
RHI_VIEW* dx12_buffers_create_view(const RHI_VIEW_DESC* const desc);
RHI_BUFFER* dx12_buffers_create_indices(const RHI_INDEX_BUFFER_DESC* const desc);
RHI_BUFFER* dx12_buffers_create_vertices(const RHI_VERTEX_BUFFER_DESC* const desc);

template <typename T>
T* dx12_buffers_create_2d(const RHI_BUFFER_2D_DESC* const desc)
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
	else if (buffer_type == buffer_type_rt_bvh) {
		flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	}
	//if (desc->memory_type == buffer_memory_type_shared_rw) {
	//	flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	//}
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

	ASSERT_SUCCESS(hr);
	ASSERT_PTR(i_resource);

	T* buffer_impl = new T();
	ASSERT_PTR(buffer_impl);
	buffer_impl->length = desc->length;
	buffer_impl->format = desc->format;
	buffer_impl->current_state = D3D12_RESOURCE_STATE_COMMON;
	buffer_impl->stride = desc->stride;
	buffer_impl->set_handle(i_resource);

	return buffer_impl;
}

template <typename T>
T* dx12_buffers_create(const RHI_BUFFER_DESC* const desc) {

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
	return dx12_buffers_create_2d<T>(&desc_2d);
}
inline RHI_BUFFER* dx12_buffers_create_raw(const RHI_BUFFER_DESC* const desc) {
	return dx12_buffers_create<DX_BUFFER>(desc);
}
#endif