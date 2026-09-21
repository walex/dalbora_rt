#include "vk_fence.hpp"

RHI_FENCE* vk_fence_create(const RHI_FENCE_DESC* const desc) {

	VkFence fence = VK_NULL_HANDLE;

	VkFenceCreateInfo fence_info{};
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_info.pNext = nullptr;

	// create a signaled fence
	fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	vkCreateFence(*static_cast<VK_DEVICE*>(desc->device), &fence_info, nullptr, &fence);
	ASSERT_PTR(fence);

	VK_FENCE* result = new VK_FENCE();
	result->set_handle(fence);
	result->parent_device = static_cast<VK_DEVICE*>(desc->device);
	return result;
}
