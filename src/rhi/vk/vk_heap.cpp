#include "vk_heap.hpp"

// D3D12_DESCRIPTOR_RANGE -> VkDescriptorSetLayoutBinding
// ID3D12RootSignature -> VkPipelineLayout


uint32_t memory_resource_vk_find_memory_type(VkPhysicalDevice physical_device, uint32_t type_filter, 
	VkMemoryPropertyFlags properties) {

	VkPhysicalDeviceMemoryProperties mem_properties;
	vkGetPhysicalDeviceMemoryProperties(physical_device, &mem_properties);

	for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++) {
		if ((type_filter & (1 << i)) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}
	throw std::runtime_error("Memory type not found");
}

RHI_MEMORY_DESCRIPTOR* memory_resource_vk_descriptor_table(const VK_DEVICE* const device_impl,
	const memory_descriptor_type heap_type,
	const size_t slots_size,
	const bool shader_visible) {

	ASSERT_PTR(device_impl);

	VkBufferUsageFlags usage_flags = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
	switch (heap_type) {
	case memory_descriptor_type_buffer:
		usage_flags |= VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;
		break;
	case memory_descriptor_type_sampler:
		usage_flags |= VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT;
		break;
	case memory_descriptor_type_dx_rtv:
	case memory_descriptor_type_dx_dsv:
		return nullptr; // RTV and DSV are not supported in Vulkan
	default:
		throw std::exception("heap type not supported");
	}
	
	VkPhysicalDeviceDescriptorHeapPropertiesEXT heap_props{};
	heap_props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_HEAP_PROPERTIES_EXT;
	vkGetPhysicalDeviceProperties2(device_impl->physical_device, reinterpret_cast<VkPhysicalDeviceProperties2*>(&heap_props));
	
	size_t max_size = std::max(heap_props.bufferDescriptorSize, heap_props.imageDescriptorSize);
	size_t max_alignment = std::max(heap_props.bufferDescriptorAlignment, heap_props.imageDescriptorAlignment);
	size_t slots_stride = (max_size + max_alignment - 1) & ~(max_alignment - 1);

	VkBufferCreateInfo heapInfo{};
	heapInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	heapInfo.size = slots_stride * slots_size;
	heapInfo.usage = usage_flags;

	VkBuffer heap_memory;
	vkCreateBuffer(*device_impl, &heapInfo, nullptr, &heap_memory);
	
	VkMemoryRequirements mem_requirements;
	vkGetBufferMemoryRequirements(*device_impl, heap_memory, &mem_requirements);

	VkMemoryAllocateFlagsInfo allocFlagsInfo{};
	allocFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
	allocFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT; // for pointer on GPU

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.pNext = &allocFlagsInfo; // Connect the flags
	allocInfo.allocationSize = mem_requirements.size; // The size returned
	allocInfo.memoryTypeIndex = memory_resource_vk_find_memory_type(device_impl->physical_device, mem_requirements.memoryTypeBits,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	// GPU handle
	VkDeviceMemory heap_memory_device;
	if (vkAllocateMemory(*device_impl, &allocInfo, nullptr, &heap_memory_device) != VK_SUCCESS) {
		throw std::runtime_error("Error allocating memory for Descriptor Heap");
	}
	vkBindBufferMemory(*device_impl, heap_memory, heap_memory_device, 0);

	// get gpu buffer address
	VkBufferDeviceAddressInfo addressInfo{};
	addressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
	addressInfo.pNext = nullptr;
	addressInfo.buffer = heap_memory;
	uint64_t base_gpu_address = vkGetBufferDeviceAddress(*device_impl, &addressInfo);
	ASSERT_EXPR(base_gpu_address != 0);

	VK_MEMORY_DESCRIPTOR* result = new VK_MEMORY_DESCRIPTOR();
	ASSERT_PTR(result);
	result->set_handle(heap_memory);
	result->memory_device = heap_memory_device;
	result->descriptor_count = slots_size;
	result->descriptor_size = slots_stride;
	result->parent_device = *device_impl;
	result->map();
	result->cpu_handle = reinterpret_cast<uint64_t>(result->mapped_memory);
	result->gpu_handle = base_gpu_address;
	return result;
}

RHI_MEMORY_DESCRIPTOR* vk_memory_resource_create(const RHI_MEMORY_RESOURCE_DESC* const desc) {

	ASSERT_PTR(desc);
	ASSERT_PTR(desc->device);

	switch (desc->type) {
	case memory_resource_type_descriptor_table:
		return memory_resource_vk_descriptor_table(static_cast<const VK_DEVICE*>(desc->device), desc->descriptor_desc.type,
			desc->descriptor_desc.count, desc->descriptor_desc.shader_visible);
	case memory_resource_type_pool:
		throw std::exception("memory resource type pool not implemented");
	default:
		throw std::exception("memory resource type not supported");
	}
}

RHI_MEMORY_DESCRIPTOR_SLOT* vk_memory_resource_get_descriptor(const RHI_MEMORY_DESCRIPTOR* const heap, const size_t index) {

	ASSERT_EXPR(index < heap->descriptor_count);
	ASSERT_PTR(heap);

	VK_MEMORY_DESCRIPTOR* heap_impl = static_cast<VK_MEMORY_DESCRIPTOR*>(const_cast<RHI_MEMORY_DESCRIPTOR*>(heap));
	uint8_t* base_heap_ptr = static_cast<uint8_t*>(heap_impl->mapped_memory);

	VK_MEMORY_DESCRIPTOR_SLOT* result = new VK_MEMORY_DESCRIPTOR_SLOT();
	ASSERT_PTR(result);
	result->slot_id = index;
	result->descriptor_size = heap_impl->descriptor_size;
	result->descriptor_count = 1;
	result->memory_ptr = base_heap_ptr + (index * heap_impl->descriptor_size);
	result->cpu_handle = reinterpret_cast<uint64_t>(result->memory_ptr);
	result->gpu_handle = heap_impl->gpu_handle + (index * heap_impl->descriptor_size);
	return result;
}

void vk_memory_resource_write_image_descriptor(const VK_DEVICE* const device_impl, const RHI_MEMORY_DESCRIPTOR_SLOT* const slot, 
	const VkImageViewCreateInfo* const image_view_info) {
	
	ASSERT_PTR(device_impl);
	ASSERT_PTR(slot);
	ASSERT_PTR(image_view_info);

	VkImageDescriptorInfoEXT image_heap_info{};
	image_heap_info.sType = VK_STRUCTURE_TYPE_IMAGE_DESCRIPTOR_INFO_EXT;
	image_heap_info.pNext = nullptr;
	image_heap_info.pView = image_view_info;
	image_heap_info.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkResourceDescriptorInfoEXT resource_info{};
	resource_info.sType = VK_STRUCTURE_TYPE_RESOURCE_DESCRIPTOR_INFO_EXT;
	resource_info.pNext = nullptr;
	resource_info.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	resource_info.data.pImage = &image_heap_info;

	VkHostAddressRangeEXT target_address{};
	target_address.address = reinterpret_cast<void*>(slot->cpu_handle);
	target_address.size = slot->descriptor_size;

	if (vkWriteResourceDescriptorsEXT(
		*device_impl,
		1,                // resourceCount
		&resource_info,   // pResources
		&target_address   // pDescriptors
	) != VK_SUCCESS) {
		throw std::runtime_error("Error writing resource descriptors");
	}

}

/*
void registerNewStorageBuffer(VkBuffer myBuffer) {
	// 1. Solicitamos un slot nuevo. La clase calcula y nos da AMBOS punteros al instante.
	DescriptorHandle newSlot = resourceHeap.allocateSlot();

	// 2. Preparamos la información del recurso que queremos guardar
	VkDescriptorAddressInfoEXT bufferAddressInfo{};
	bufferAddressInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_ADDRESS_INFO_EXT;
	bufferAddressInfo.buffer = myBuffer;
	bufferAddressInfo.offset = 0;
	bufferAddressInfo.range = VK_WHOLE_SIZE;

	VkDescriptorGetInfoEXT bufferGetInfo{};
	bufferGetInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT;
	bufferGetInfo.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	bufferGetInfo.data.pStorageBuffer = &bufferAddressInfo;

	// 3. USAMOS EL PUNTERO DE CPU: Escribimos los bytes del descriptor directamente en la dirección calculada
	vkGetDescriptorEXT(device, &bufferGetInfo, resourceDescriptorSize, newSlot.cpuAddress);

	// 4. USAMOS EL PUNTERO DE GPU E ÍNDICE: 
	// Guardamos 'newSlot.gpuAddress' o 'newSlot.index' en la estructura de tu entidad/objeto
	// para pasárselo al shader mediante Push Constants en el momento del renderizado.
	myMesh.descriptorIndex = newSlot.index;
	myMesh.gpuDescriptorAddress = newSlot.gpuAddress; // Por si necesitas bindings específicos por rango
}
*/