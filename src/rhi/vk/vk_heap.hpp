#ifndef __vk_heap_hpp__
#define __vk_heap_hpp__

#include "vk_rhi.hpp"

RHI_MEMORY_DESCRIPTOR* vk_memory_resource_create(const RHI_MEMORY_RESOURCE_DESC* const desc);
RHI_MEMORY_DESCRIPTOR_SLOT* vk_memory_resource_get_descriptor(const RHI_MEMORY_DESCRIPTOR* const heap, const size_t index);
int32_t vk_memory_resource_find_memory_type(VkPhysicalDevice physical_device, uint32_t type_filter,
	VkMemoryPropertyFlags properties);

void vk_memory_resource_write_image_descriptor(const VK_DEVICE* const device_impl, const RHI_MEMORY_DESCRIPTOR_SLOT* const slot,
	const VkImageViewCreateInfo* const image_view_info);
void vk_memory_resource_write_buffer_view_descriptor(const VK_DEVICE* const device_impl, const RHI_MEMORY_DESCRIPTOR_SLOT* const slot,
	const VkBufferViewCreateInfo* const buffer_view_info
);
void vk_memory_resource_write_uniform_buffer_descriptor(const VK_DEVICE* const device_impl, const RHI_MEMORY_DESCRIPTOR_SLOT* const slot,
	const VkDescriptorBufferInfo* const buffer_info);
void vk_memory_resource_write_constant_buffer_view_descriptor(const VK_DEVICE* const device_impl, const RHI_MEMORY_DESCRIPTOR_SLOT* const slot,
	const VkDescriptorBufferInfo* const cbv_view_info);
void vk_memory_resource_write_acceleration_structure_descriptor(const VK_DEVICE* const device_impl, const RHI_MEMORY_DESCRIPTOR_SLOT* const slot,
	const VkAccelerationStructureKHR* const acceleration_structure);

#endif