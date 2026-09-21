#ifndef __vk_heap_hpp__
#define __vk_heap_hpp__

#include "vk_rhi.hpp"

RHI_MEMORY_DESCRIPTOR* vk_memory_resource_create(const RHI_MEMORY_RESOURCE_DESC* const desc);
RHI_MEMORY_DESCRIPTOR_SLOT* vk_memory_resource_get_descriptor(const RHI_MEMORY_DESCRIPTOR* const heap, const size_t index);
#endif