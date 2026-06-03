#ifndef __dx12_command_queue_hpp__
#define __dx12_command_queue_hpp__

#include "dx12_rhi.hpp"

RHI_COMMAND_QUEUE* dx12_command_queue_create_for_render(const RHI_COMMAND_QUEUE_DESC* const desc);
RHI_COMMAND_QUEUE* dx12_command_queue_create_for_compute(const RHI_COMMAND_QUEUE_DESC* const desc);
RHI_COMMAND_QUEUE* dx12_command_queue_create_for_copy(const RHI_COMMAND_QUEUE_DESC* const desc);
void dx12_command_queue_execute(RHI_COMMAND_QUEUE* const command_queue, const bool wait_completion, fptr_command_queue_on_execute callback);
void dx12_command_queue_sync(RHI_COMMAND_QUEUE* const command_queue);

#endif