#ifndef __dx12_index_buffer_hpp__
#define __dx12_index_buffer_hpp__

#include "dx12_rhi.hpp"

std::unique_ptr<RHI_OBJECT> dx12_index_buffer_create(const RHI_INDEX_BUFFER_DESC& desc);
void dx12_index_buffer_update(RHI_OBJECT& index_buffer, const void* data, size_t size);

#endif