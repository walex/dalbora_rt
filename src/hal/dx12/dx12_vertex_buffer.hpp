#ifndef __dx12_vertex_buffer_hpp__
#define __dx12_vertex_buffer_hpp__

#include "dx12_hal.hpp"

std::unique_ptr<HAL_OBJECT> dx12_create_vertex_buffer(const HAL_VERTEX_BUFFER_DESC& vb_desc);

#endif