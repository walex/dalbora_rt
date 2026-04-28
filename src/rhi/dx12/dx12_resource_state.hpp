#ifndef __dx12_resource_state_hpp__
#define __dx12_resource_state_hpp__

#include "dx12_rhi.hpp"

void dx12_resource_state_transition(RHI_OBJECT& command_buffer, RHI_RESOURCE& resource, resource_state new_state);

#endif