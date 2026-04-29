#ifndef __dx12_command_queue_hpp__
#define __dx12_command_queue_hpp__

#include "dx12_rhi.hpp"

std::unique_ptr<RHI_OBJECT> dx12_command_queue_create_for_render(const RHI_COMMAND_QUEUE_DESC& queue_desc);
std::unique_ptr<RHI_OBJECT> dx12_command_queue_create_for_compute(const RHI_COMMAND_QUEUE_DESC& queue_desc);
std::unique_ptr<RHI_OBJECT> dx12_command_queue_create_for_copy(const RHI_COMMAND_QUEUE_DESC& queue_desc);
void dx12_command_queue_execute_list(RHI_COMMAND_BUUFER_LIST& command_buffers, bool sync = false);
void dx12_command_queue_execute_list_synchronized(RHI_COMMAND_BUUFER_LIST& command_buffers);
void dx12_command_queue_wait(RHI_COMMAND_BUUFER_LIST& command_buffers);
void dx12_command_queue_execute(RHI_OBJECT& queue, RHI_OBJECT& cmd_buffer, bool sync = false);
void dx12_command_queue_execute_synchronized(RHI_OBJECT& queue, RHI_OBJECT& cmd_buffer);
#endif