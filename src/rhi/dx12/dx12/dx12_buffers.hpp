#ifndef __dx12_buffers_hpp__
#define __dx12_buffers_hpp__

#include "dx12_rhi.hpp"

std::unique_ptr<RHI_OBJECT> dx12_buffers_create(const RHI_BUFFER_DESC& desc);
std::unique_ptr<RHI_OBJECT> dx12_buffers_create_depth(const RHI_DEPTH_BUFFER_DESC& desc);
void dx12_buffers_upload(RHI_TRANSFER_BUFFER_DESC& desc, const bool sync = false);
void dx12_buffers_upload_synchronized(RHI_TRANSFER_BUFFER_DESC& desc);
void dx12_buffers_download_synchronized(RHI_TRANSFER_BUFFER_DESC& desc);

#endif