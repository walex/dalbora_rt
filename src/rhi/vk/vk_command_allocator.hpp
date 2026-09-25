#ifndef __vk_command_allocator_hpp__
#define __vk_command_allocator_hpp__

#include "vk_rhi.hpp"

RHI_COMMAND_ALLOCATOR* vk_command_allocator_create(const RHI_DEVICE* const device, const queue_type type);

#endif