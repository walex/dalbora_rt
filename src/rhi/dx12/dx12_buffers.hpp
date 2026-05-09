#ifndef __dx12_buffers_hpp__
#define __dx12_buffers_hpp__

#include "dx12_rhi.hpp"

std::unique_ptr<RHI_BUFFER> dx12_buffers_create_raw(const RHI_BUFFER_DESC& desc);
std::unique_ptr<RHI_DEPTH_BUFFER> dx12_buffers_create_depth(const RHI_DEPTH_BUFFER_DESC& desc);
std::unique_ptr<RHI_CONSTANT_BUFFER> dx12_buffers_create_constant(const RHI_BUFFER_DESC& desc);

void dx12_buffers_copy_buffer(RHI_COMMAND_BUFFER& command_buffer, RHI_BUFFER& src_buffer,
	RHI_BUFFER& dest_buffer);
void dx12_buffers_copy_buffer_region(RHI_COMMAND_BUFFER& command_buffer, RHI_BUFFER& src_buffer,
	size_t offset_src, RHI_BUFFER& dest_buffer, size_t offset_dest,
	size_t length);
void dx12_buffers_gpu_upload_region(RHI_COMMAND_BUFFER& command_buffer, RHI_BUFFER& cpu_buffer,
	RHI_BUFFER& gpu_buffer, size_t offset_src, size_t offset_dest, size_t length);
void dx12_buffers_gpu_upload(RHI_COMMAND_BUFFER& command_buffer, RHI_BUFFER& cpu_buffer, RHI_BUFFER& gpu_buffer);
void dx12_buffers_gpu_download_region(RHI_COMMAND_BUFFER& command_buffer, RHI_BUFFER& cpu_buffer,
	RHI_BUFFER& gpu_buffer, size_t offset_src, size_t offset_dest, size_t length);
void dx12_buffers_gpu_download(RHI_COMMAND_BUFFER& command_buffer, RHI_BUFFER& cpu_buffer, RHI_BUFFER& gpu_buffer);
void dx12_buffers_map_write(RHI_BUFFER& cpu_buffer, RHI_VOID_PTR data,
	size_t offset, size_t length);
void dx12_buffers_map_read(RHI_BUFFER& cpu_buffer, RHI_VOID_PTR& data,
	size_t offset, size_t length);

#endif