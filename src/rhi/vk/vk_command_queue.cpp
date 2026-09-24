#include "vk_command_queue.hpp"
#include "vk_fence.hpp"

RHI_COMMAND_QUEUE* vk_command_queue_create(const RHI_COMMAND_QUEUE_DESC* const desc) {
	
	ASSERT_PTR(desc);
	ASSERT_PTR(desc->device);

	VK_DEVICE* vk_device = static_cast<VK_DEVICE*>(desc->device);
	ASSERT_PTR(vk_device);
	VkDevice i_device = *vk_device;

	uint32_t family_index;
	switch (desc->type) {
	case queue_type_graphics:
		family_index = vk_device->graphics_queue_family_index;
		break;
	case queue_type_compute:
		family_index = vk_device->compute_queue_family_index;
		break;
	case queue_type_copy:
		family_index = vk_device->copy_queue_family_index;
		break;
	default:
		throw std::exception("Invalid queue type");
	}
	
	VkSemaphore semaphore = VK_NULL_HANDLE;
	VkSemaphoreCreateInfo timeline_semaphore_info{};
	timeline_semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	vkCreateSemaphore(i_device, &timeline_semaphore_info, nullptr, &semaphore);
	ASSERT_PTR(semaphore);

	VkQueue queue = VK_NULL_HANDLE;
	// '0' index if only one queue
	vkGetDeviceQueue(*vk_device, family_index, 0, &queue);
	ASSERT_PTR(queue);

	VK_COMMAND_QUEUE* vk_command_queue = new VK_COMMAND_QUEUE();
	vk_command_queue->set_handle(queue);
	vk_command_queue->parent_device = vk_device;
	vk_command_queue->timeline_semaphore = semaphore;
	vk_command_queue->type = desc->type;

	RHI_FENCE_DESC fence_desc;
	fence_desc.device = desc->device;
	fence_desc.flags = fence_flags_none;
	fence_desc.initial_value = 0;

	vk_command_queue->fence.reset(vk_fence_create(&fence_desc));
	ASSERT_PTR(vk_command_queue->fence);
	return vk_command_queue;
}

bool command_queue_vk_execute(const VkQueue i_cmd_queue, const VkFence fence, 
	const bool wait_completion, VkSubmitInfo& submit_info, 
	fptr_command_queue_on_execute callback) {
	
	std::vector<RHI_COMMAND_BUFFER*> command_buffer_list;
	callback(static_cast<RHI_VOID_PTR>(i_cmd_queue), &command_buffer_list);
	size_t list_size = command_buffer_list.size();
	if (list_size > 0) {
		std::vector<VkCommandBuffer> native_list(list_size);
		for (int i = 0; i < list_size; i++)
			native_list[i] = *static_cast<VK_COMMAND_BUFFER*>(command_buffer_list[i]);

		submit_info.commandBufferCount = static_cast<uint32_t>(native_list.size());
		submit_info.pCommandBuffers = native_list.data();

		if (vkQueueSubmit(i_cmd_queue, 1, &submit_info, fence) != VK_SUCCESS) {
			throw std::runtime_error("Error submitting command buffer to the queue");
		}
	}
	return list_size > 0;
}

void command_queue_vk_sync_init(RHI_COMMAND_QUEUE* const command_queue, VkSubmitInfo& submit_info
	, VkFence fence) {

	const VkDevice i_device = *static_cast<const VK_DEVICE*>(static_cast<VK_COMMAND_QUEUE*>(command_queue)->parent_device);
	ASSERT_PTR(i_device);

	VkTimelineSemaphoreSubmitInfo time_line_submit_info{};

	VkSemaphore i_timeline_semaphore = *static_cast<const VK_FENCE*>(command_queue->fence.get());
	ASSERT_PTR(i_timeline_semaphore);

	vkWaitForFences(i_device, 1, &fence, VK_TRUE, UINT64_MAX);
	vkResetFences(i_device, 1, &fence);

	time_line_submit_info.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
	time_line_submit_info.signalSemaphoreValueCount = 1;
	time_line_submit_info.pSignalSemaphoreValues = &command_queue->fence_counter;

	submit_info.pNext = &time_line_submit_info;
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores = &i_timeline_semaphore;
}

void command_queue_vk_sync_end(RHI_COMMAND_QUEUE* const command_queue) {

	const VkDevice i_device = *static_cast<const VK_DEVICE*>(static_cast<VK_COMMAND_QUEUE*>(command_queue)->parent_device);
	ASSERT_PTR(i_device);

	VkSemaphore i_timeline_semaphore = *static_cast<const VK_FENCE*>(command_queue->fence.get());
	ASSERT_PTR(i_timeline_semaphore);

	uint64_t completed_value = 0;
	vkGetSemaphoreCounterValue(i_device, i_timeline_semaphore, &completed_value);

	if (completed_value < command_queue->fence_counter) {
		VkSemaphoreWaitInfo waitInfo{};
		waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
		waitInfo.semaphoreCount = 1;
		waitInfo.pSemaphores = &i_timeline_semaphore;
		waitInfo.pValues = &command_queue->fence_counter;

		vkWaitSemaphores(i_device, &waitInfo, UINT64_MAX);
	}
}

void vk_command_queue_execute(RHI_COMMAND_QUEUE* const command_queue, 
	const bool wait_completion, fptr_command_queue_on_execute callback) {
	
	ASSERT_PTR(command_queue);

	VkQueue i_cmd_queue = *static_cast<VK_COMMAND_QUEUE*>(command_queue);
	ASSERT_PTR(i_cmd_queue);
	
	VkSubmitInfo submit_info{};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	if (wait_completion == true) {
		VkFence fence = *static_cast<VK_FENCE*>(command_queue->fence.get());
		command_queue_vk_sync_init(command_queue, submit_info, fence);
		if (command_queue_vk_execute(i_cmd_queue, fence, wait_completion, submit_info, callback) == true) {
			command_queue_vk_sync_end(command_queue);
		}
	}
	else {
		command_queue_vk_execute(i_cmd_queue, VK_NULL_HANDLE, wait_completion, submit_info, callback);
	}
}

