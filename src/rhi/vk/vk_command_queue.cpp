#include "vk_command_queue.hpp"

void vk_command_queue_get_queue_family_indices(const VkPhysicalDevice physical_device, uint32_t& graphics_family_index,
    uint32_t& compute_family_index) {

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queueFamilyCount, queueFamilies.data());

    for (uint32_t i = 0; i < queueFamilyCount; i++) {

        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            graphics_family_index = i;
        }
        // ¿Esta familia soporta comandos de cómputo puro?
        if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
            compute_family_index = i; 
        }

        if (graphics_family_index != -1 && compute_family_index  != -1) {
            break;
        }
    }
}