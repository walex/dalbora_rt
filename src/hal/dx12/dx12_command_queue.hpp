#ifndef __dx12_command_queue_hpp__
#define __dx12_command_queue_hpp__

#include "dx12_hal.hpp"

std::unique_ptr<HAL_OBJECT> dx12_create_graphics_command_queue(const HAL_COMMAND_QUEUE_DESC& queue_desc);
std::unique_ptr<HAL_OBJECT> dx12_create_compute_command_queue(const HAL_COMMAND_QUEUE_DESC& queue_desc);
std::unique_ptr<HAL_OBJECT> dx12_create_copy_command_queue(const HAL_COMMAND_QUEUE_DESC& queue_desc);
void dx12_command_queue_execute_command_buffers(HAL_COMMAND_BUUFER_LIST& command_buffers);
void dx12_command_queue_wait_command_buffers(HAL_COMMAND_BUUFER_LIST& command_buffers);

#endif