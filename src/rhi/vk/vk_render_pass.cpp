#include "vk_render_pass.hpp"


RHI_RENDER_PASS* vk_render_pass_create(const RHI_RENDER_PASS_DESC* const desc) {

	ASSERT_PTR(desc);
	ASSERT_PTR(desc->device);

	VK_RENDER_PASS* result = new VK_RENDER_PASS();
	ASSERT_PTR(result);
	result->device = desc->device;

	return result;
}

void vk_render_pass_execute_raster_mode(const RHI_RENDER_PASS* const render_pass, RHI_COMMAND_BUFFER* const command_buffer,
	fptr_render_pass_on_execute callback) {

	ASSERT_PTR(render_pass);

	VK_IMAGE_VIEW* render_target_view = static_cast<VK_IMAGE_VIEW*>(render_pass->render_target_view.get());
	ASSERT_PTR(render_target_view);

	VK_COMMAND_BUFFER* command_buffer_impl = static_cast<VK_COMMAND_BUFFER*>(command_buffer);
	ASSERT_PTR(command_buffer_impl);

	// Se definen los attachments directamente al grabar los comandos
	VkRenderingAttachmentInfo colorAttachment{};
	colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	colorAttachment.imageView = *render_target_view;
	colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.clearValue = { {{0.1f, 0.2f, 0.4f, 1.0f}} };

	const RHI_VIEWPORT& vp = render_pass->view_port;

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
	image_barrier.image = *static_cast<VK_TEXTURE_2D*>(render_target_view->buffer.get());

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

	// ==========================================
	// AQUÍ YA PUEDES LLAMAR SEGURO A:
	// vkCmdBeginRendering(commandBuffer, &renderingInfo);
	// ==========================================


	VkRenderingInfo renderingInfo{};
	renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
	renderingInfo.renderArea = { {static_cast<int32_t>(vp.x), static_cast<int32_t>(vp.y)},
		{static_cast<uint32_t>(vp.width), static_cast<uint32_t>(vp.height)} };
	renderingInfo.layerCount = 1;
	renderingInfo.colorAttachmentCount = 1;
	renderingInfo.pColorAttachments = &colorAttachment;

	vkCmdBeginRendering(*command_buffer_impl, &renderingInfo);

	if (callback) {
		callback();
	}

	vkCmdEndRendering(*command_buffer_impl);

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
	present_barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;                  // El diseño obligatorio para el monitor

	present_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	present_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	present_barrier.image = *static_cast<VK_TEXTURE_2D*>(render_target_view->buffer.get());                                  // Pasa aquí tu VkImage (la 0x2b...)

	present_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	present_barrier.subresourceRange.baseMipLevel = 0;
	present_barrier.subresourceRange.levelCount = 1;
	present_barrier.subresourceRange.baseArrayLayer = 0;
	present_barrier.subresourceRange.layerCount = 1;

	// 2. Agrupar la barrera en la información de dependencias
	dependency_info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	dependency_info.pNext = nullptr;
	dependency_info.imageMemoryBarrierCount = 1;
	dependency_info.pImageMemoryBarriers = &present_barrier;

	// 3. Grabar de forma segura en tu Command Buffer
	vkCmdPipelineBarrier2(*command_buffer_impl, &dependency_info);
}