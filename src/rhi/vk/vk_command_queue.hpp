#ifndef __vk_command_queue_hpp__
#define __vk_command_queue_hpp__

#include "vk_rhi.hpp"

void vk_command_queue_get_queue_family_indices(const VkPhysicalDevice physical_device, uint32_t& graphics_family_index,
    uint32_t& compute_family_index);

#endif