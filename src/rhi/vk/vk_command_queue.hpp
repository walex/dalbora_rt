#ifndef __vk_command_queue_hpp__
#define __vk_command_queue_hpp__

#include "vk_rhi.hpp"

RHI_COMMAND_QUEUE* vk_command_queue_create_for_render(const RHI_COMMAND_QUEUE_DESC* const desc);
RHI_COMMAND_QUEUE* vk_command_queue_create_for_compute(const RHI_COMMAND_QUEUE_DESC* const desc);
RHI_COMMAND_QUEUE* vk_command_queue_create_for_copy(const RHI_COMMAND_QUEUE_DESC* const desc);
void vk_command_queue_execute(RHI_COMMAND_QUEUE* const command_queue, const bool wait_completion, fptr_command_queue_on_execute callback);

#endif