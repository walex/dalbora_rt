#ifndef __dx12_vertex_buffer_hpp__
#define __dx12_vertex_buffer_hpp__

#include "dx12_rhi.hpp"

std::unique_ptr<RHI_OBJECT> dx12_create_vertex_buffer(const RHI_VERTEX_BUFFER_DESC& vb_desc);

#endif