#include "vk_fence.hpp"

RHI_FENCE* vk_fence_create(const RHI_FENCE_DESC* const desc) {

	VkSemaphore semaphore = VK_NULL_HANDLE;
	VkSemaphoreTypeCreateInfo semaphore_type_info{};
	semaphore_type_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
	semaphore_type_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
	semaphore_type_info.initialValue = 0;

	VkSemaphoreCreateInfo timeline_semaphore_info{};
	timeline_semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	timeline_semaphore_info.pNext = &semaphore_type_info;
	vkCreateSemaphore(*static_cast<VK_DEVICE*>(desc->device), &timeline_semaphore_info, nullptr, &semaphore);
	ASSERT_PTR(semaphore);


	VK_FENCE* result = new VK_FENCE();
	result->set_handle(semaphore);
	result->parent_device = static_cast<VK_DEVICE*>(desc->device);
	return result;
}
