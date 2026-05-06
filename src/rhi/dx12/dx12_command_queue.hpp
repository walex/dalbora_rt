#ifndef __dx12_command_queue_hpp__
#define __dx12_command_queue_hpp__

#include "dx12_rhi.hpp"

std::unique_ptr<RHI_COMMAND_QUEUE> dx12_command_queue_create_for_render(const RHI_COMMAND_QUEUE_DESC& queue_desc);
std::unique_ptr<RHI_COMMAND_QUEUE> dx12_command_queue_create_for_compute(const RHI_COMMAND_QUEUE_DESC& queue_desc);
std::unique_ptr<RHI_COMMAND_QUEUE> dx12_command_queue_create_for_copy(const RHI_COMMAND_QUEUE_DESC& queue_desc);
void dx12_command_queue_exec(RHI_COMMAND_QUEUE& command_queue, bool wait_completion, fptr_command_queue_on_execute callback);
void dx12_command_queue_wait(RHI_COMMAND_QUEUE& command_queue);

#endif