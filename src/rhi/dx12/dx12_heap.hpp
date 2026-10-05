#ifndef __dx12_heap_hpp__
#define __dx12_heap_hpp__

#include "dx12_rhi.hpp"

RHI_MEMORY_DESCRIPTOR* dx12_memory_resource_create(const RHI_MEMORY_RESOURCE_DESC* const desc);

RHI_MEMORY_DESCRIPTOR_SLOT* dx12_memory_resource_get_descriptor(const RHI_MEMORY_DESCRIPTOR* const heap, const size_t index);

bool dx12_memory_resource_check_type(const DX_DEVICE* const device_impl, const buffer_memory_type memory_type);

#endif