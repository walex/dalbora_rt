#ifndef __dx12_buffers_hpp__
#define __dx12_buffers_hpp__

#include "dx12_rhi.hpp"

RHI_DEPTH_BUFFER* dx12_buffers_create_depth(const RHI_DEPTH_BUFFER_DESC* const desc);
RHI_CONSTANT_BUFFER* dx12_buffers_create_constant(const RHI_BUFFER_DESC* const desc);
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
RHI_INDEX_BUFFER* dx12_buffers_create_indices(const RHI_INDEX_BUFFER_DESC* const desc);
RHI_VERTEX_BUFFER* dx12_buffers_create_vertices(const RHI_VERTEX_BUFFER_DESC* const desc);
#endif