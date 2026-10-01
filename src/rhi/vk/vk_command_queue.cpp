#include "vk_command_queue.hpp"
#include "vk_fence.hpp"

void command_queue_vk_execute(VK_COMMAND_QUEUE* command_queue, fptr_command_queue_on_execute callback) {
	
	VkSubmitInfo submit_info{};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkQueue i_cmd_queue = *static_cast<const VK_COMMAND_QUEUE*>(command_queue);
	ASSERT_PTR(i_cmd_queue);
	
	std::vector<RHI_COMMAND_BUFFER*> command_buffer_list;
	callback(static_cast<RHI_VOID_PTR>(i_cmd_queue), &command_buffer_list);
	size_t list_size = command_buffer_list.size();
	if (list_size > 0) {
		std::vector<VkCommandBuffer> native_list(list_size);
		for (int i = 0; i < list_size; i++)
			native_list[i] = *static_cast<VK_COMMAND_BUFFER*>(command_buffer_list[i]);

		submit_info.commandBufferCount = static_cast<uint32_t>(native_list.size());
		submit_info.pCommandBuffers = native_list.data();

		if (vkQueueSubmit(i_cmd_queue, 1, &submit_info, VK_NULL_HANDLE) != VK_SUCCESS) {
			throw std::runtime_error("Error submitting command buffer to the queue");
		}
	}
}


RHI_COMMAND_QUEUE* vk_command_queue_create(const RHI_COMMAND_QUEUE_DESC* const desc) {

	ASSERT_PTR(desc);
	ASSERT_PTR(desc->device);

	VK_DEVICE* vk_device = static_cast<VK_DEVICE*>(desc->device);
	ASSERT_PTR(vk_device);
	VkDevice i_device = *vk_device;

	uint32_t family_index;
	switch (desc->type) {
	case queue_type_graphics:
		family_index = vk_device->queue_family_index[queue_type_graphics];
		break;
	case queue_type_compute:
		family_index = vk_device->queue_family_index[queue_type_compute];
		break;
	case queue_type_copy:
		family_index = vk_device->queue_family_index[queue_type_copy];
		break;
	default:
		throw std::exception("Invalid queue type");
	}
	
	VkQueue queue = VK_NULL_HANDLE;
	// '0' index if only one queue
	vkGetDeviceQueue(*vk_device, family_index, 0, &queue);
	ASSERT_PTR(queue);

	VK_COMMAND_QUEUE* vk_command_queue = new VK_COMMAND_QUEUE();
	vk_command_queue->set_handle(queue);
	vk_command_queue->parent_device = vk_device;
	vk_command_queue->type = desc->type;

	RHI_FENCE_DESC fence_desc;
	fence_desc.device = desc->device;
	fence_desc.flags = fence_flags_none;
	fence_desc.initial_value = 0;

	vk_command_queue->fence.reset(vk_fence_create(&fence_desc));
	ASSERT_PTR(vk_command_queue->fence);
	return vk_command_queue;
}

uint64_t vk_command_queue_execute(RHI_COMMAND_QUEUE* const command_queue, 
	const bool wait_completion, fptr_command_queue_on_execute callback) {
	
	ASSERT_PTR(command_queue);
	ASSERT_PTR(command_queue->fence);

	VK_COMMAND_QUEUE* cmd_queue_impl = static_cast<VK_COMMAND_QUEUE*>(command_queue);
	ASSERT_PTR(cmd_queue_impl);

	const VkDevice i_device = *static_cast<const VK_DEVICE*>(cmd_queue_impl->parent_device);
	ASSERT_PTR(i_device);

	VkQueue i_cmd_queue = *cmd_queue_impl;
	ASSERT_PTR(i_cmd_queue);

	VK_FENCE* i_fence = static_cast<VK_FENCE*>(cmd_queue_impl->fence.get());
	ASSERT_PTR(i_fence);
	VkSemaphore i_timeline_semaphore = *i_fence;
	ASSERT_PTR(i_timeline_semaphore);

	// 1. Obtener los command buffers desde la aplicación
	std::vector<RHI_COMMAND_BUFFER*> command_buffer_list;
	callback(static_cast<RHI_VOID_PTR>(i_cmd_queue), &command_buffer_list);
	size_t list_size = command_buffer_list.size();

	if (list_size > 0) {
		// Preparar la lista de command buffers en el formato moderno VkCommandBufferSubmitInfo
		std::vector<VkCommandBufferSubmitInfo> cmd_buffer_infos(list_size);
		for (size_t i = 0; i < list_size; i++) {
			VkCommandBufferSubmitInfo& info = cmd_buffer_infos[i];
			info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
			info.pNext = nullptr;
			info.commandBuffer = *static_cast<VK_COMMAND_BUFFER*>(command_buffer_list[i]);
			info.deviceMask = 0; // 0 para GPUs individuales estándar
		}

		// Incrementar el contador de sincronización
		cmd_queue_impl->fence->counter++;
		uint64_t target_value = cmd_queue_impl->fence->counter;

		// 2. Configurar la señalización del Timeline Semaphore directamente en su estructura nativa
		VkSemaphoreSubmitInfo signal_semaphore_info{};
		signal_semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
		signal_semaphore_info.pNext = nullptr;
		signal_semaphore_info.semaphore = i_timeline_semaphore;
		signal_semaphore_info.value = target_value;  // <-- El valor del Timeline se asigna directamente aquí
		signal_semaphore_info.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT; // Señalar al terminar todo el trabajo
		signal_semaphore_info.deviceIndex = 0;

		std::vector<VkSemaphoreSubmitInfo> wait_info_native(cmd_queue_impl->sync_objects_count);
		for (size_t i = 0; i < cmd_queue_impl->sync_objects_count; i++) {
			VK_PIPELINE_STAGE_SYNC& wait_info = cmd_queue_impl->sync_objects[i];
			VkSemaphoreSubmitInfo& wait_info_native_ref = wait_info_native[i];
			wait_info_native_ref.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
			wait_info_native_ref.pNext = nullptr;
			wait_info_native_ref.semaphore = wait_info.semaphore;
			wait_info_native_ref.stageMask = vk_pipeline_stages[wait_info.stage];
		}

		// 3. Estructura moderna de envío obligatoria desde Vulkan 1.3/1.4
		VkSubmitInfo2 submit_info2{};
		submit_info2.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
		submit_info2.pNext = nullptr; // Ya no requiere de extensiones pNext para el Timeline
		submit_info2.commandBufferInfoCount = static_cast<uint32_t>(cmd_buffer_infos.size());
		submit_info2.pCommandBufferInfos = cmd_buffer_infos.data();
		submit_info2.signalSemaphoreInfoCount = 1;
		submit_info2.pSignalSemaphoreInfos = &signal_semaphore_info;
		submit_info2.waitSemaphoreInfoCount = static_cast<uint32_t>(wait_info_native.size());
		submit_info2.pWaitSemaphoreInfos = wait_info_native.data();

		// 4. Enviar a la cola usando la función moderna vkQueueSubmit2
		if (vkQueueSubmit2(i_cmd_queue, 1, &submit_info2, VK_NULL_HANDLE) != VK_SUCCESS) {
			throw std::runtime_error("Error submitting command buffer to the queue via vkQueueSubmit2");
		}
	}

	cmd_queue_impl->sync_objects_count = 0;

	// 5. Gestión de espera en la CPU (Se mantiene igual de seguro)
	uint64_t completed_value = 0;
	vkGetSemaphoreCounterValue(i_device, i_timeline_semaphore, &completed_value);

	if (wait_completion == true && completed_value < cmd_queue_impl->fence->counter) {
		VkSemaphoreWaitInfo waitInfo{};
		waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
		waitInfo.semaphoreCount = 1;
		waitInfo.pSemaphores = &i_timeline_semaphore;
		waitInfo.pValues = &cmd_queue_impl->fence->counter;

		vkWaitSemaphores(i_device, &waitInfo, UINT64_MAX);
		completed_value = cmd_queue_impl->fence->counter;
	}

	return completed_value;
}

