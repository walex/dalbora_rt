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
    result->command_queue_type = desc->command_queue->type;
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

    ASSERT_PTR(command_buffer);
    ASSERT_PTR(vb);

    VK_COMMAND_BUFFER* vk_command_buffer = static_cast<VK_COMMAND_BUFFER*>(command_buffer);

    // 2. Vincular el Vertex Buffer
    VkBuffer vertexBuffers[] = { *static_cast<VK_BUFFER*>(vb) };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(*vk_command_buffer, 0, 1, vertexBuffers, offsets);

    static constexpr VkIndexType index_type[] {
        VK_INDEX_TYPE_UINT8,
        VK_INDEX_TYPE_UINT16,
        VK_INDEX_TYPE_UINT32
    };
   
    if (ib != nullptr) {
        uint32_t idx = static_cast<uint32_t>(ib->stride * 0.5f);
        ASSERT_EXPR(idx >=0 && idx < 3);
        vkCmdBindIndexBuffer(*vk_command_buffer, *static_cast<VK_BUFFER*>(ib), 0, index_type[idx]);
        vkCmdDrawIndexed(*vk_command_buffer, static_cast<uint32_t>(ib->length / ib->stride), 1, 0, 0, 0);
    }
    else {
		vkCmdDraw(*vk_command_buffer, static_cast<uint32_t>(vb->length / vb->stride), 1, 0, 0);
    }


    
}

void vk_command_buffer_ray_trace(RHI_COMMAND_BUFFER* const command_buffer, RHI_TEXTURE_2D* const render_target,
	const RHI_SBT_TABLE* const sbt) {

    ASSERT_PTR(command_buffer);
    ASSERT_PTR(render_target);
    ASSERT_PTR(sbt);

    VK_COMMAND_BUFFER* command_buffer_impl = static_cast<VK_COMMAND_BUFFER*>(command_buffer);
    VK_TEXTURE_2D* render_target_impl = static_cast<VK_TEXTURE_2D*>(render_target);
    const VK_SBT_TABLE* sbt_impl = static_cast<const VK_SBT_TABLE*>(sbt);
    ASSERT_PTR(command_buffer_impl->parent_device);
    ASSERT_EXPR(command_buffer_impl->handle != VK_NULL_HANDLE);
    ASSERT_EXPR(render_target_impl->handle != VK_NULL_HANDLE);
    ASSERT_EXPR(sbt_impl->handle != VK_NULL_HANDLE);
    ASSERT_EXPR(sbt->record_size > 0 && sbt->ray_gen_size >= sbt->record_size);
    ASSERT_EXPR(render_target->width > 0 && render_target->width <= UINT32_MAX);
    ASSERT_EXPR(render_target->height > 0 && render_target->height <= UINT32_MAX);

    VkBufferDeviceAddressInfo address_info{};
    address_info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    address_info.buffer = *sbt_impl;
    const VkDeviceAddress table_address = vkGetBufferDeviceAddress(*command_buffer_impl->parent_device, &address_info);
    ASSERT_EXPR(table_address != 0);

    VkStridedDeviceAddressRegionKHR ray_gen{};
    ray_gen.deviceAddress = table_address + sbt->ray_gen_offset;
    ray_gen.stride = sbt->record_size;
    ray_gen.size = sbt->record_size;
    VkStridedDeviceAddressRegionKHR miss{};
    if (sbt->miss_size > 0) {
        miss.deviceAddress = table_address + sbt->miss_offset;
        miss.stride = sbt->record_size;
        miss.size = sbt->miss_size;
    }
    VkStridedDeviceAddressRegionKHR hit_group{};
    if (sbt->hit_group_size > 0) {
        hit_group.deviceAddress = table_address + sbt->hit_group_offset;
        hit_group.stride = sbt->record_size;
        hit_group.size = sbt->hit_group_size;
    }
    VkStridedDeviceAddressRegionKHR callable{};

    const VkImageLayout previous_layout = render_target_impl->mip_zero_layout;
    VkImageMemoryBarrier2 image_barrier{};
    image_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    image_barrier.srcStageMask = previous_layout == VK_IMAGE_LAYOUT_UNDEFINED
        ? VK_PIPELINE_STAGE_2_NONE : VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    image_barrier.srcAccessMask = previous_layout == VK_IMAGE_LAYOUT_UNDEFINED
        ? VK_ACCESS_2_NONE : VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
    image_barrier.dstStageMask = VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR;
    image_barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT;
    image_barrier.oldLayout = previous_layout;
    image_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    image_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_barrier.image = *render_target_impl;
    image_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_barrier.subresourceRange.levelCount = 1;
    image_barrier.subresourceRange.layerCount = 1;
    VkDependencyInfo dependency{};
    dependency.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    dependency.imageMemoryBarrierCount = 1;
    dependency.pImageMemoryBarriers = &image_barrier;
    vkCmdPipelineBarrier2(*command_buffer_impl, &dependency);

    vkCmdTraceRaysKHR(*command_buffer_impl, &ray_gen, &miss, &hit_group, &callable,
        static_cast<uint32_t>(render_target->width), static_cast<uint32_t>(render_target->height), 1);

    image_barrier.srcStageMask = VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR;
    image_barrier.srcAccessMask = VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT;
    image_barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    image_barrier.dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
    image_barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    image_barrier.newLayout = previous_layout == VK_IMAGE_LAYOUT_UNDEFINED ? VK_IMAGE_LAYOUT_GENERAL : previous_layout;
    vkCmdPipelineBarrier2(*command_buffer_impl, &dependency);
    render_target_impl->mip_zero_layout = image_barrier.newLayout;
}	

void vk_command_buffer_copy_texture(RHI_COMMAND_BUFFER* const command_buffer, RHI_TEXTURE_2D* const dest_texture,
	const RHI_TEXTURE_2D* const src_texture) {
    ASSERT_PTR(command_buffer);
    ASSERT_PTR(dest_texture);
    ASSERT_PTR(src_texture);
    ASSERT_EXPR(dest_texture != src_texture);
    ASSERT_EXPR(dest_texture->width == src_texture->width && dest_texture->height == src_texture->height);
    ASSERT_EXPR(dest_texture->width > 0 && dest_texture->width <= UINT32_MAX);
    ASSERT_EXPR(dest_texture->height > 0 && dest_texture->height <= UINT32_MAX);
    ASSERT_EXPR(!src_texture->mip_maps.empty() && src_texture->mip_maps.size() == dest_texture->mip_maps.size());
    ASSERT_EXPR(src_texture->mip_maps.size() <= UINT32_MAX / 2);

    VK_COMMAND_BUFFER* command_buffer_impl = static_cast<VK_COMMAND_BUFFER*>(command_buffer);
    auto* src_image = dynamic_cast<VK_NON_DISPATCHABLE_HANDLE<VkImage>*>(const_cast<RHI_TEXTURE_2D*>(src_texture));
    auto* dest_image = dynamic_cast<VK_NON_DISPATCHABLE_HANDLE<VkImage>*>(dest_texture);
    ASSERT_PTR(src_image);
    ASSERT_PTR(dest_image);
    ASSERT_EXPR(command_buffer_impl->handle != VK_NULL_HANDLE);
    ASSERT_EXPR(src_image->handle != VK_NULL_HANDLE && dest_image->handle != VK_NULL_HANDLE);
    ASSERT_EXPR(src_image->parent_device == command_buffer_impl->parent_device && dest_image->parent_device == command_buffer_impl->parent_device);
    ASSERT_EXPR(src_image->handle != dest_image->handle);

    auto get_layout = [](RHI_TEXTURE_2D* texture, size_t mip) -> VkImageLayout& {
        if (auto* image = dynamic_cast<VK_TEXTURE_2D*>(texture)) {
            if (mip == 0)
                return image->mip_zero_layout;
            image->other_mip_layouts.resize(texture->mip_maps.size() - 1, VK_IMAGE_LAYOUT_UNDEFINED);
            return image->other_mip_layouts[mip - 1];
        }
        auto* image = dynamic_cast<VK_SWAP_CHAIN_TEXTURE_2D*>(texture);
        ASSERT_PTR(image);
        ASSERT_EXPR(mip == 0);
        return image->mip_zero_layout;
    };

    auto* src_swap_chain = dynamic_cast<VK_SWAP_CHAIN_TEXTURE_2D*>(const_cast<RHI_TEXTURE_2D*>(src_texture));
    auto* dest_swap_chain = dynamic_cast<VK_SWAP_CHAIN_TEXTURE_2D*>(dest_texture);
    ASSERT_EXPR(!src_swap_chain || (src_swap_chain->image_usage & VK_IMAGE_USAGE_TRANSFER_SRC_BIT));
    ASSERT_EXPR(!dest_swap_chain || (dest_swap_chain->image_usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT));

    const size_t mip_count = src_texture->mip_maps.size();
    std::vector<VkImageMemoryBarrier2> barriers(mip_count * 2);
    std::vector<VkImageCopy> regions(mip_count);
    for (size_t mip = 0; mip < mip_count; ++mip) {
        const auto& src_mip = src_texture->mip_maps[mip];
        const auto& dest_mip = dest_texture->mip_maps[mip];
        ASSERT_EXPR(src_mip.width == dest_mip.width && src_mip.height == dest_mip.height);
        ASSERT_EXPR(src_mip.width > 0 && src_mip.width <= UINT32_MAX && src_mip.height > 0 && src_mip.height <= UINT32_MAX);
        regions[mip].srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, static_cast<uint32_t>(mip), 0, 1 };
        regions[mip].dstSubresource = regions[mip].srcSubresource;
        regions[mip].extent = { static_cast<uint32_t>(src_mip.width), static_cast<uint32_t>(src_mip.height), 1 };

        for (size_t i = 0; i < 2; ++i) {
            RHI_TEXTURE_2D* texture = i == 0 ? const_cast<RHI_TEXTURE_2D*>(src_texture) : dest_texture;
            VkImageLayout& layout = get_layout(texture, mip);
            VkImageMemoryBarrier2& barrier = barriers[mip * 2 + i];
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
            barrier.srcStageMask = layout == VK_IMAGE_LAYOUT_UNDEFINED ? VK_PIPELINE_STAGE_2_NONE : VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
            barrier.srcAccessMask = layout == VK_IMAGE_LAYOUT_UNDEFINED ? VK_ACCESS_2_NONE : VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
            barrier.dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
            barrier.dstAccessMask = i == 0 ? VK_ACCESS_2_TRANSFER_READ_BIT : VK_ACCESS_2_TRANSFER_WRITE_BIT;
            barrier.oldLayout = layout;
            barrier.newLayout = i == 0 ? VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL : VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = i == 0 ? src_image->handle : dest_image->handle;
            barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, static_cast<uint32_t>(mip), 1, 0, 1 };
        }
    }

    VkDependencyInfo dependency{};
    dependency.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    dependency.imageMemoryBarrierCount = static_cast<uint32_t>(barriers.size());
    dependency.pImageMemoryBarriers = barriers.data();
    vkCmdPipelineBarrier2(*command_buffer_impl, &dependency);
    vkCmdCopyImage(*command_buffer_impl, src_image->handle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        dest_image->handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast<uint32_t>(regions.size()), regions.data());

    for (size_t mip = 0; mip < mip_count; ++mip) {
        for (size_t i = 0; i < 2; ++i) {
            RHI_TEXTURE_2D* texture = i == 0 ? const_cast<RHI_TEXTURE_2D*>(src_texture) : dest_texture;
            VkImageLayout& layout = get_layout(texture, mip);
            VkImageMemoryBarrier2& barrier = barriers[mip * 2 + i];
            barrier.srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
            barrier.srcAccessMask = i == 0 ? VK_ACCESS_2_TRANSFER_READ_BIT : VK_ACCESS_2_TRANSFER_WRITE_BIT;
            barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
            barrier.dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
            barrier.oldLayout = barrier.newLayout;
            barrier.newLayout = layout == VK_IMAGE_LAYOUT_UNDEFINED
                ? ((i == 0 ? src_swap_chain : dest_swap_chain) ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_GENERAL)
                : layout;
            layout = barrier.newLayout;
        }
    }
    vkCmdPipelineBarrier2(*command_buffer_impl, &dependency);
}