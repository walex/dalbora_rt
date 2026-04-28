#ifndef __dx12_vertex_buffer_hpp__
#define __dx12_vertex_buffer_hpp__

#include "dx12_rhi.hpp"

std::unique_ptr<RHI_OBJECT> dx12_vertex_buffer_create(const RHI_VERTEX_BUFFER_DESC& vb_desc);
void dx12_vertex_buffer_update(RHI_RESOURCE& vertex_buffer, const void* data, size_t size);

#endif