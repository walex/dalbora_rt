#ifndef __dx12_command_queue_hpp__
#define __dx12_command_queue_hpp__

#include "dx12_rhi.hpp"

std::unique_ptr<RHI_OBJECT> dx12_create_graphics_command_queue(const RHI_COMMAND_QUEUE_DESC& queue_desc);
std::unique_ptr<RHI_OBJECT> dx12_create_compute_command_queue(const RHI_COMMAND_QUEUE_DESC& queue_desc);
std::unique_ptr<RHI_OBJECT> dx12_create_copy_command_queue(const RHI_COMMAND_QUEUE_DESC& queue_desc);
void dx12_command_queue_execute_command_buffers(RHI_COMMAND_BUUFER_LIST& command_buffers, bool sync = false);
void dx12_command_queue_execute_command_buffers_synchronized(RHI_COMMAND_BUUFER_LIST& command_buffers);
void dx12_command_queue_wait_command_buffers(RHI_COMMAND_BUUFER_LIST& command_buffers);
void dx12_command_queue_execute_single_command_buffer(RHI_COMMAND_QUEUE& queue, RHI_OBJECT& cmd_buffer, bool sync = false);
void dx12_command_queue_execute_single_command_buffer_synchronized(RHI_COMMAND_QUEUE& queue, RHI_OBJECT& cmd_buffer);
#endif