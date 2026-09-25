#ifndef __dx12_command_allocator_hpp__
#define __dx12_command_allocator_hpp__

#include "dx12_rhi.hpp"

RHI_COMMAND_ALLOCATOR* dx12_command_allocator_create(const RHI_DEVICE* const device, const queue_type type);

#endif