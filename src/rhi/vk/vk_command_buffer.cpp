#include "vk_command_buffer.hpp"

RHI_COMMAND_BUFFER* vk_command_buffer_create(const RHI_COMMAND_BUFFER_DESC* const desc) {

    ASSERT_PTR(desc);
    ASSERT_PTR(desc->device);
    ASSERT_PTR(desc->command_queue);
    ASSERT_PTR(desc->command_allocator);
    
	VK_DEVICE* device_impl = static_cast<VK_DEVICE*>(desc->device);
    VkCommandPool commnad_pool = *static_cast<VK_COMMAND_ALLOCATOR*>(desc->command_allocator);
    VkCommandBuffer commandBuffer;

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commnad_pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(*device_impl, &allocInfo, &commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("¡Error al asignar el Command Buffer!");
    }
	VK_COMMAND_BUFFER* result = new VK_COMMAND_BUFFER();
	result->set_handle(commandBuffer);
	result->command_pool = commnad_pool;
    result->parent_device = device_impl;
	result->command_allocator = desc->command_allocator;
	return result;
}

void vk_command_buffer_record(RHI_COMMAND_BUFFER* const command_buffer, fptr_command_buffer_on_record callback) {

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    VK_COMMAND_BUFFER* vk_command_buffer = static_cast<VK_COMMAND_BUFFER*>(command_buffer);
    if (vkBeginCommandBuffer(*vk_command_buffer, &beginInfo) == VK_SUCCESS) {

        if (callback)
            callback(command_buffer);

        vkEndCommandBuffer(*vk_command_buffer);
    }
}

void vk_command_buffer_draw_triangle_list(RHI_COMMAND_BUFFER* const command_buffer, RHI_BUFFER* const vb,
	RHI_BUFFER* const ib) {
}

void vk_command_buffer_ray_trace(RHI_COMMAND_BUFFER* const command_buffer, RHI_TEXTURE_2D* const render_target,
	const RHI_SBT_TABLE* const sbt) {
}	

void vk_command_buffer_copy_texture(RHI_COMMAND_BUFFER* const command_buffer, RHI_TEXTURE_2D* const dest_texture,
	const RHI_TEXTURE_2D* const src_texture) {
}