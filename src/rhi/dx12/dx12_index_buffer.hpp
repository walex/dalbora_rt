#ifndef __dx12_index_buffer_hpp__
#define __dx12_index_buffer_hpp__

#include "dx12_rhi.hpp"

std::unique_ptr<RHI_OBJECT> dx12_create_index_buffer(const RHI_INDEX_BUFFER_DESC& ib_desc);

#endif