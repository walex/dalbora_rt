#include "vk_render_pass.hpp"


RHI_RENDER_PASS* vk_render_pass_create(const RHI_RENDER_PASS_DESC* const desc) {

	ASSERT_PTR(desc);
	ASSERT_PTR(desc->device);

	VK_RENDER_PASS* result = new VK_RENDER_PASS();
	ASSERT_PTR(result);
	result->device = desc->device;

	return result;
}

void vk_render_pass_execute_rt_mode(const RHI_RENDER_PASS* const render_pass, RHI_COMMAND_BUFFER* const command_buffer,
	fptr_render_pass_on_execute callback) {

	ASSERT_PTR(render_pass);
	ASSERT_PTR(render_pass->device);
	ASSERT_PTR(render_pass->render_target_view);
	ASSERT_PTR(render_pass->pipeline);
	ASSERT_PTR(render_pass->pipeline->layout);
	ASSERT_PTR(command_buffer);
	ASSERT_PTR(command_buffer->buffer_memory_descriptor);

	VK_COMMAND_BUFFER* command_buffer_impl = static_cast<VK_COMMAND_BUFFER*>(command_buffer);
	VK_RT_PIPELINE* pipeline_impl = static_cast<VK_RT_PIPELINE*>(render_pass->pipeline.get());
	ASSERT_EXPR(command_buffer_impl->handle != VK_NULL_HANDLE);
	ASSERT_EXPR(pipeline_impl->handle != VK_NULL_HANDLE);

	VK_MEMORY_DESCRIPTOR* buffer_memory_descriptor = static_cast<VK_MEMORY_DESCRIPTOR*>(command_buffer_impl->buffer_memory_descriptor.get());
	VkBindHeapInfoEXT resource_bind_info{};
	resource_bind_info.sType = VK_STRUCTURE_TYPE_BIND_HEAP_INFO_EXT;
	resource_bind_info.heapRange.address = buffer_memory_descriptor->gpu_handle;
	resource_bind_info.heapRange.size = buffer_memory_descriptor->descriptor_capacity;
	resource_bind_info.reservedRangeSize = buffer_memory_descriptor->reserved_range;
	resource_bind_info.reservedRangeOffset = buffer_memory_descriptor->reserved_offset;
	vkCmdBindResourceHeapEXT(*command_buffer_impl, &resource_bind_info);

	if (command_buffer_impl->sampler_memory_descriptor) {
		VK_MEMORY_DESCRIPTOR* sampler_memory_descriptor = static_cast<VK_MEMORY_DESCRIPTOR*>(command_buffer_impl->sampler_memory_descriptor.get());
		VkBindHeapInfoEXT sampler_bind_info{};
		sampler_bind_info.sType = VK_STRUCTURE_TYPE_BIND_HEAP_INFO_EXT;
		sampler_bind_info.heapRange.address = sampler_memory_descriptor->gpu_handle;
		sampler_bind_info.heapRange.size = sampler_memory_descriptor->descriptor_capacity;
		sampler_bind_info.reservedRangeSize = sampler_memory_descriptor->reserved_range;
		sampler_bind_info.reservedRangeOffset = sampler_memory_descriptor->reserved_offset;
		vkCmdBindSamplerHeapEXT(*command_buffer_impl, &sampler_bind_info);
	}

	vkCmdBindPipeline(*command_buffer_impl, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, *pipeline_impl);
	if (callback)
		callback();
}

void vk_render_pass_execute_raster_mode(const RHI_RENDER_PASS* const render_pass, RHI_COMMAND_BUFFER* const command_buffer,
	fptr_render_pass_on_execute callback) {

	ASSERT_PTR(render_pass);

	VK_IMAGE_VIEW* render_target_view = static_cast<VK_IMAGE_VIEW*>(render_pass->render_target_view.get());
	ASSERT_PTR(render_target_view);
	auto* render_target_image = dynamic_cast<VK_NON_DISPATCHABLE_HANDLE<VkImage>*>(render_target_view->buffer.get());
	ASSERT_PTR(render_target_image);

	VK_COMMAND_BUFFER* command_buffer_impl = static_cast<VK_COMMAND_BUFFER*>(command_buffer);
	ASSERT_PTR(command_buffer_impl);
	const bool has_pipeline = render_pass->pipeline.get() != nullptr;

	// Se definen los attachments directamente al grabar los comandos
	VkRenderingAttachmentInfo colorAttachment{};
	colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	colorAttachment.imageView = *render_target_view;
	colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.clearValue = { {{0.1f, 0.2f, 0.4f, 1.0f}} };

	const RHI_VIEWPORT& vp = render_pass->view_port;

	UINT heap_count = 0;
	if (command_buffer_impl->buffer_memory_descriptor) {

		VK_MEMORY_DESCRIPTOR* buffer_memory_descriptor = static_cast<VK_MEMORY_DESCRIPTOR*>(command_buffer_impl->buffer_memory_descriptor.get());
		VkDescriptorBufferBindingInfoEXT& bindingInfo = command_buffer_impl->heaps[heap_count];

		// 1. Configurar la información de enlace para el Heap de Recursos
		VkBindHeapInfoEXT resourceBindInfo{};
		resourceBindInfo.sType = VK_STRUCTURE_TYPE_BIND_HEAP_INFO_EXT;
		resourceBindInfo.pNext = nullptr;

		// El rango completo del buffer que actúa como heap (Dirección de GPU y Tamaño total)
		resourceBindInfo.heapRange.address = command_buffer_impl->buffer_memory_descriptor->gpu_handle;
		resourceBindInfo.heapRange.size = command_buffer_impl->buffer_memory_descriptor->descriptor_capacity;

		// El área reservada por el driver (estos límites los consultas previamente de las propiedades físicas)
		resourceBindInfo.reservedRangeSize = buffer_memory_descriptor->reserved_range;     // Tamaño reservado
		resourceBindInfo.reservedRangeOffset = buffer_memory_descriptor->reserved_offset;

		// 2. Vincular el heap de recursos al command buffer
		vkCmdBindResourceHeapEXT(*command_buffer_impl, &resourceBindInfo);

	}

	if (command_buffer_impl->sampler_memory_descriptor) {
		VK_MEMORY_DESCRIPTOR* sampler_memory_descriptor = static_cast<VK_MEMORY_DESCRIPTOR*>(command_buffer_impl->sampler_memory_descriptor.get());
		VkBindHeapInfoEXT samplerBindInfo{};
		samplerBindInfo.sType = VK_STRUCTURE_TYPE_BIND_HEAP_INFO_EXT;
		samplerBindInfo.heapRange.address = sampler_memory_descriptor->gpu_handle;
		samplerBindInfo.heapRange.size = sampler_memory_descriptor->descriptor_capacity;
		samplerBindInfo.reservedRangeSize = sampler_memory_descriptor->reserved_range;
		samplerBindInfo.reservedRangeOffset = sampler_memory_descriptor->reserved_offset;

		vkCmdBindSamplerHeapEXT(*command_buffer_impl, &samplerBindInfo);
		heap_count++;
	}


	// 1. Vincular la pipeline gráfica
	if (has_pipeline) {
		vkCmdBindPipeline(*command_buffer_impl, VK_PIPELINE_BIND_POINT_GRAPHICS, *static_cast<const VK_RASTER_PIPELINE*>(render_pass->pipeline.get()));
	}

	{
		// 1. Configurar la barrera de transición con la API moderna (Synchronization2)
		VkImageMemoryBarrier2 image_barrier{};
		image_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
		image_barrier.pNext = nullptr;

		// Etapas de sincronización de operaciones
		image_barrier.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;         // Desde el inicio del pipeline...
		image_barrier.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT; // ...hasta la etapa de escritura de color

		// Accesos de memoria
		image_barrier.srcAccessMask = VK_ACCESS_2_NONE;                          // No requiere accesos previos
		image_barrier.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;    // Se preparará para escribir color

		// El cambio crítico de diseño (Layout Transition) que soluciona tu error
		image_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;                    // El diseño actual reportado por el error
		image_barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;     // El diseño que espera vkCmdBeginRendering

		image_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		image_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		image_barrier.image = render_target_image->handle;

		// Detalles de la subresource
		image_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		image_barrier.subresourceRange.baseMipLevel = 0;
		image_barrier.subresourceRange.levelCount = 1;
		image_barrier.subresourceRange.baseArrayLayer = 0;
		image_barrier.subresourceRange.layerCount = 1;

		// 2. Agrupar la barrera en la estructura de dependencia
		VkDependencyInfo dependency_info{};
		dependency_info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		dependency_info.pNext = nullptr;
		dependency_info.imageMemoryBarrierCount = 1;
		dependency_info.pImageMemoryBarriers = &image_barrier;

		// 3. Grabar la barrera en tu Command Buffer moderno justo antes del renderizado
		vkCmdPipelineBarrier2(*command_buffer_impl, &dependency_info);

	}

	// ==========================================
	// AQUÍ YA PUEDES LLAMAR SEGURO A:
	// vkCmdBeginRendering(commandBuffer, &renderingInfo);
	// ==========================================

	// Set view port and scissor
	VkViewport viewport{};
	viewport.x = static_cast<float>(vp.x);
	viewport.y = static_cast<float>(vp.y);
	viewport.width = static_cast<float>(vp.width);
	viewport.height = static_cast<float>(vp.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(*command_buffer_impl, 0, 1, &viewport);

	VkRenderingInfo renderingInfo{};
	renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
	renderingInfo.renderArea = { {static_cast<int32_t>(vp.x), static_cast<int32_t>(vp.y)},
		{static_cast<uint32_t>(vp.width), static_cast<uint32_t>(vp.height)} };
	renderingInfo.layerCount = 1;
	renderingInfo.colorAttachmentCount = 1;
	renderingInfo.pColorAttachments = &colorAttachment;
	vkCmdSetScissor(*command_buffer_impl, 0, 1, &renderingInfo.renderArea);


	vkCmdBeginRendering(*command_buffer_impl, &renderingInfo);

	if (callback && has_pipeline) {
		callback();
	}

	vkCmdEndRendering(*command_buffer_impl);

	{
		VkImageMemoryBarrier2 present_barrier{};
		present_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
		present_barrier.pNext = nullptr;

		// ETAPAS DEL PIPELINE (Cuándo se detiene la GPU)
		present_barrier.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT; // Espera que termine de escribir color
		present_barrier.dstStageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;          // Se libera al final de la cola

		// ACCESOS DE MEMORIA (Qué operaciones de caché se limpian)
		present_barrier.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;        // Limpia la caché de escritura de color
		present_barrier.dstAccessMask = VK_ACCESS_2_NONE;                              // No requiere accesos GPU posteriores

		// DISEÑOS DE IMAGEN (El cambio físico que soluciona tu error de Presentación)
		present_barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;          // El diseño que usaba en el BeginRendering
		present_barrier.newLayout = dynamic_cast<VK_SWAP_CHAIN_TEXTURE_2D*>(render_target_view->buffer.get())
			? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_GENERAL;

		present_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		present_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		present_barrier.image = render_target_image->handle;

		present_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		present_barrier.subresourceRange.baseMipLevel = 0;
		present_barrier.subresourceRange.levelCount = 1;
		present_barrier.subresourceRange.baseArrayLayer = 0;
		present_barrier.subresourceRange.layerCount = 1;

		// 2. Agrupar la barrera en la información de dependencias
		VkDependencyInfo dependency_info{};
		dependency_info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		dependency_info.pNext = nullptr;
		dependency_info.imageMemoryBarrierCount = 1;
		dependency_info.pImageMemoryBarriers = &present_barrier;

		// 3. Grabar de forma segura en tu Command Buffer
		vkCmdPipelineBarrier2(*command_buffer_impl, &dependency_info);
		VK_TEXTURE_2D* texture = dynamic_cast<VK_TEXTURE_2D*>(render_target_view->buffer.get());
		if (texture)
			texture->mip_zero_layout = present_barrier.newLayout;
		VK_SWAP_CHAIN_TEXTURE_2D* swap_chain_texture = dynamic_cast<VK_SWAP_CHAIN_TEXTURE_2D*>(render_target_view->buffer.get());
		if (swap_chain_texture)
			swap_chain_texture->mip_zero_layout = present_barrier.newLayout;
	}
	if (callback && !has_pipeline)
		callback();
}