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
	colorAttachment.clearValue = { {{0.0f, 0.2f, 0.4f, 1.0f}} };

	const RHI_VIEWPORT& vp = render_pass->view_port;

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
}