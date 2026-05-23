#ifndef __dx12_heap_hpp__
#define __dx12_heap_hpp__

#include "dx12_rhi.hpp"

DX_HEAP* dx12_heap_create(const DX_DEVICE* const device_impl, resource_type resource_type, const size_t slot_count, const bool shader_visible);

#endif