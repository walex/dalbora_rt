#ifndef __dx12_resource_state_hpp__
#define __dx12_resource_state_hpp__

#include "dx12_hal.hpp"

void dx12_resource_state_transition(HAL_OBJECT& command_buffer, HAL_RESOURCE& resource, resource_state old_state, resource_state new_state);

#endif