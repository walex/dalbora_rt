#ifndef __dx12_index_buffer_hpp__
#define __dx12_index_buffer_hpp__

#include "dx12_hal.hpp"

std::unique_ptr<HAL_OBJECT> dx12_create_index_buffer(const HAL_INDEX_BUFFER_DESC& ib_desc);

#endif