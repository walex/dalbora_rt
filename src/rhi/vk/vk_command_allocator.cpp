#include "vk_command_allocator.hpp"

RHI_COMMAND_ALLOCATOR* vk_command_allocator_create(const RHI_DEVICE* const device, const queue_type type) {

	const VK_DEVICE* vk_device = static_cast<const VK_DEVICE*>(device);
	ASSERT_PTR(vk_device);

	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = vk_device->queue_family_index[type];
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	VkCommandPool i_cmd_allocator = VK_NULL_HANDLE;
	if (vkCreateCommandPool(*vk_device, &poolInfo, nullptr, &i_cmd_allocator) != VK_SUCCESS) {
		throw std::runtime_error("¡Error al crear el command pool!");
	}

	VK_COMMAND_ALLOCATOR* result = new VK_COMMAND_ALLOCATOR();
	result->set_handle(i_cmd_allocator);
	result->parent_device = vk_device;

	return result;
}
