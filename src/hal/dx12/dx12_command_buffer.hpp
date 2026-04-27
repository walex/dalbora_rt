#ifndef __dx12_command_buffer_hpp__
#define __dx12_command_buffer_hpp__

#include "dx12_hal.hpp"

std::unique_ptr<HAL_OBJECT> dx12_create_command_buffer(const HAL_COMMAND_BUFFER_DESC& cb_desc);

#endif