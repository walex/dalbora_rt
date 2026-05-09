#ifndef __dx12_index_buffer_hpp__
#define __dx12_index_buffer_hpp__

#include "dx12_rhi.hpp"

std::unique_ptr<RHI_BUFFER> dx12_index_buffer_create(const RHI_INDEX_BUFFER_DESC& desc);

#endif