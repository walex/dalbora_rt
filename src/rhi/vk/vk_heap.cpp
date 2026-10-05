#include "vk_heap.hpp"

// D3D12_DESCRIPTOR_RANGE -> VkDescriptorSetLayoutBinding
// ID3D12RootSignature -> VkPipelineLayout

void calculate_max_descriptor_layout_properties_vk(VkPhysicalDevice physical_device, size_t* slots_stride_out, 
	size_t* slot_alignment_out) {

	ASSERT_PTR(slots_stride_out);
	ASSERT_PTR(slot_alignment_out);


	// 1. Estructura de propiedades de Descriptor Buffer
	VkPhysicalDeviceDescriptorBufferPropertiesEXT desc_buffer_props{};
	desc_buffer_props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_PROPERTIES_EXT;

	// 2. Extensión PhysicalDeviceProperties2 para consultar estructuras en pNext
	VkPhysicalDeviceProperties2 device_props_2{};
	device_props_2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	device_props_2.pNext = &desc_buffer_props;

	vkGetPhysicalDeviceProperties2(physical_device, &device_props_2);

	// 3. Obtener el TAMAÑO máximo entre todos los tipos de descriptores soportados
	size_t max_size = std::max({
		desc_buffer_props.samplerDescriptorSize,                    // Sampler
		desc_buffer_props.combinedImageSamplerDescriptorSize,       // Combined Image-Sampler
		desc_buffer_props.sampledImageDescriptorSize,               // SRV Texture
		desc_buffer_props.storageImageDescriptorSize,               // UAV Texture
		desc_buffer_props.uniformTexelBufferDescriptorSize,        // SRV Texel Buffer
		desc_buffer_props.storageTexelBufferDescriptorSize,        // UAV Texel Buffer
		desc_buffer_props.uniformBufferDescriptorSize,             // CBV
		desc_buffer_props.storageBufferDescriptorSize,             // UAV / SRV Buffer
		desc_buffer_props.accelerationStructureDescriptorSize       // Ray Tracing BVH
		});

	// 4. La alineación requerida para el offset/stride dentro del buffer
	size_t alignment = static_cast<size_t>(desc_buffer_props.descriptorBufferOffsetAlignment);

	// 5. Redondear max_size al múltiplo más cercano de la alineación
	size_t slots_stride = (max_size + alignment - 1) & ~(alignment - 1);

	// 7. Calcular el Stride redondeando max_size al múltiplo más cercano de max_alignment
	*slots_stride_out = (max_size + alignment - 1) & ~(alignment - 1);
	*slot_alignment_out = alignment;
}

int32_t vk_memory_resource_find_memory_type(VkPhysicalDevice physical_device, uint32_t type_filter, 
	VkMemoryPropertyFlags properties) {

	VkPhysicalDeviceMemoryProperties mem_properties;
	vkGetPhysicalDeviceMemoryProperties(physical_device, &mem_properties);

	for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++) {
		if ((type_filter & (1 << i)) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}
	return -1;
}

RHI_MEMORY_DESCRIPTOR* memory_resource_descriptor_table_vk(const VK_DEVICE* const device_impl,
	const memory_descriptor_type heap_type,
	const size_t slots_count,
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
	case memory_descriptor_type_rtv:
		usage_flags |= VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;
		break;
	case memory_descriptor_type_dsv:
		return nullptr; // DSV not supported in Vulkan
	default:
		throw std::exception("heap type not supported");
	}
	
	
	VkPhysicalDeviceDescriptorHeapPropertiesEXT heap_props{};
	VkPhysicalDeviceProperties2 device_props_2 = {};
	device_props_2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	heap_props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_HEAP_PROPERTIES_EXT;
	device_props_2.pNext = &heap_props;

	vkGetPhysicalDeviceProperties2(device_impl->physical_device, &device_props_2);
	
	size_t max_size = 0;
	size_t slot_alignment = 0;
	size_t slot_stride = 0;
	calculate_max_descriptor_layout_properties_vk(device_impl->physical_device, &slot_stride, &slot_alignment);

	VkBufferCreateInfo heapInfo{};
	heapInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	heapInfo.size = slot_stride * slots_count;
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
	allocInfo.memoryTypeIndex = vk_memory_resource_find_memory_type(device_impl->physical_device, mem_requirements.memoryTypeBits,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	ASSERT_EXPR(allocInfo.memoryTypeIndex >= 0, "Memory type not found");

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
	result->descriptor_count = slots_count;
	result->descriptor_size = slot_stride;
	result->descriptor_alignment = slot_alignment;

	result->parent_device = device_impl;
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
		return memory_resource_descriptor_table_vk(static_cast<const VK_DEVICE*>(desc->device), desc->descriptor_desc.type,
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
	result->descriptor_alignment = heap_impl->descriptor_alignment;
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
	ASSERT_EXPR((slot->cpu_handle % slot->descriptor_alignment) == 0);

	VkImageDescriptorInfoEXT image_heap_info{};
	image_heap_info.sType = VK_STRUCTURE_TYPE_IMAGE_DESCRIPTOR_INFO_EXT;
	image_heap_info.pNext = nullptr;
	image_heap_info.pView = image_view_info;
	image_heap_info.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkResourceDescriptorInfoEXT resource_info{};
	resource_info.sType = VK_STRUCTURE_TYPE_RESOURCE_DESCRIPTOR_INFO_EXT;
	resource_info.pNext = nullptr;
	resource_info.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
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

void vk_memory_resource_write_buffer_view_descriptor(
	const VK_DEVICE* const device_impl,
	const RHI_MEMORY_DESCRIPTOR_SLOT* const slot,
	const VkBufferViewCreateInfo* const buffer_view_info
) {
	ASSERT_PTR(device_impl);
	ASSERT_PTR(slot);
	ASSERT_PTR(buffer_view_info);
	ASSERT_EXPR(buffer_view_info->buffer != VK_NULL_HANDLE);
	ASSERT_EXPR((slot->cpu_handle % slot->descriptor_alignment) == 0);

	// 1. Obtener la dirección base en GPU del VkBuffer contenido en el CreateInfo
	VkBufferDeviceAddressInfo address_info{};
	address_info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
	address_info.pNext = nullptr;
	address_info.buffer = buffer_view_info->buffer;

	const VkDeviceAddress buffer_device_address = vkGetBufferDeviceAddress(*device_impl, &address_info);
	ASSERT_EXPR(buffer_device_address != 0);

	// 2. Construir la información del Texel Buffer
	VkTexelBufferDescriptorInfoEXT texel_buffer_info{};
	texel_buffer_info.sType = VK_STRUCTURE_TYPE_TEXEL_BUFFER_DESCRIPTOR_INFO_EXT;
	texel_buffer_info.pNext = nullptr;
	texel_buffer_info.format = buffer_view_info->format;
	texel_buffer_info.addressRange.address = buffer_device_address + buffer_view_info->offset;
	texel_buffer_info.addressRange.size = buffer_view_info->range;

	// 3. Determinar el tipo de descriptor evaluando las propiedades del formato
	VkFormatProperties format_props{};
	vkGetPhysicalDeviceFormatProperties(device_impl->physical_device, buffer_view_info->format, &format_props);

	const bool is_storage = (format_props.bufferFeatures & VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT) != 0;

	const VkDescriptorType descriptor_type = is_storage
		? VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER
		: VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;

	VkResourceDescriptorInfoEXT resource_info{};
	resource_info.sType = VK_STRUCTURE_TYPE_RESOURCE_DESCRIPTOR_INFO_EXT;
	resource_info.pNext = nullptr;
	resource_info.type = descriptor_type;
	resource_info.data.pTexelBuffer = &texel_buffer_info;

	// 4. Escribir el descriptor en la dirección de CPU del slot
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

void vk_memory_resource_write_uniform_buffer_descriptor(const VK_DEVICE* const device_impl,	const RHI_MEMORY_DESCRIPTOR_SLOT* const slot,
	const VkDescriptorBufferInfo* const buffer_info) {
	ASSERT_PTR(device_impl);
	ASSERT_PTR(slot);
	ASSERT_PTR(buffer_info);
	ASSERT_EXPR(buffer_info->buffer != VK_NULL_HANDLE);
	ASSERT_EXPR((slot->cpu_handle % slot->descriptor_alignment) == 0);

	// 1. Obtener la dirección física de VRAM del VkBuffer
	VkBufferDeviceAddressInfo address_info{};
	address_info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
	address_info.pNext = nullptr;
	address_info.buffer = buffer_info->buffer;

	const VkDeviceAddress base_address = vkGetBufferDeviceAddress(*device_impl, &address_info);
	ASSERT_EXPR(base_address != 0);

	// 2. Configurar el rango de dirección GPU (Dirección Base + Offset y Tamaño)
	VkDeviceAddressRangeEXT address_range{};
	address_range.address = base_address + buffer_info->offset;
	address_range.size = buffer_info->range;

	// 3. Configurar la información del recurso apuntando a pAddressRange
	VkResourceDescriptorInfoEXT resource_info{};
	resource_info.sType = VK_STRUCTURE_TYPE_RESOURCE_DESCRIPTOR_INFO_EXT;
	resource_info.pNext = nullptr;
	resource_info.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // O VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
	resource_info.data.pAddressRange = &address_range;

	VkHostAddressRangeEXT target_address{};
	target_address.address = reinterpret_cast<void*>(slot->cpu_handle);
	target_address.size = slot->descriptor_size;

	if (vkWriteResourceDescriptorsEXT(
		*device_impl,
		1,                // resourceCount
		&resource_info,   // pResources
		&target_address   // pDescriptors
	) != VK_SUCCESS) {
		throw std::runtime_error("Error writing uniform buffer resource descriptor");
	}
}

void vk_memory_resource_write_constant_buffer_view_descriptor(const VK_DEVICE* const device_impl, const RHI_MEMORY_DESCRIPTOR_SLOT* const slot,
	const VkDescriptorBufferInfo* const cbv_view_info) {
	ASSERT_PTR(device_impl);
	ASSERT_PTR(slot);
	ASSERT_PTR(cbv_view_info);
	ASSERT_EXPR(cbv_view_info->buffer != VK_NULL_HANDLE);
	ASSERT_EXPR((slot->cpu_handle % slot->descriptor_alignment) == 0);

	VkBufferDeviceAddressInfo address_info{};
	address_info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
	address_info.pNext = nullptr;
	address_info.buffer = cbv_view_info->buffer;

	const VkDeviceAddress base_address = vkGetBufferDeviceAddress(*device_impl, &address_info);
	ASSERT_EXPR(base_address != 0);

	VkDeviceAddressRangeEXT address_range{};
	address_range.address = base_address + cbv_view_info->offset;
	address_range.size = cbv_view_info->range;

	VkResourceDescriptorInfoEXT resource_info{};
	resource_info.sType = VK_STRUCTURE_TYPE_RESOURCE_DESCRIPTOR_INFO_EXT;
	resource_info.pNext = nullptr;
	resource_info.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	resource_info.data.pAddressRange = &address_range;

	VkHostAddressRangeEXT target_address{};
	target_address.address = reinterpret_cast<void*>(slot->cpu_handle);
	target_address.size = slot->descriptor_size;

	if (vkWriteResourceDescriptorsEXT(
		*device_impl,
		1,
		&resource_info,
		&target_address
	) != VK_SUCCESS) {
		throw std::runtime_error("Error writing constant buffer view descriptor");
	}
}

void vk_memory_resource_write_acceleration_structure_descriptor(const VK_DEVICE* const device_impl,	const RHI_MEMORY_DESCRIPTOR_SLOT* const slot,
	const VkAccelerationStructureKHR* const acceleration_structure) {
	ASSERT_PTR(device_impl);
	ASSERT_PTR(slot);
	ASSERT_PTR(acceleration_structure);
	ASSERT_EXPR((slot->cpu_handle % slot->descriptor_alignment) == 0);

	// 1. Obtener la dirección física de la estructura de aceleración en VRAM
	VkAccelerationStructureDeviceAddressInfoKHR address_info{};
	address_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
	address_info.accelerationStructure = *acceleration_structure;

	const VkDeviceAddress as_device_address = vkGetAccelerationStructureDeviceAddressKHR(*device_impl, &address_info);
	ASSERT_EXPR(as_device_address != 0);

	// 2. Definir el rango de dirección (para AS, el tamaño no se requiere de forma estricta, pero se pasa la dirección)
	VkDeviceAddressRangeEXT address_range{};
	address_range.address = as_device_address;
	address_range.size = 0; // O el tamaño devuelto por vkGetAccelerationStructureMemoryRequirementsKHR

	VkResourceDescriptorInfoEXT resource_info{};
	resource_info.sType = VK_STRUCTURE_TYPE_RESOURCE_DESCRIPTOR_INFO_EXT;
	resource_info.pNext = nullptr;
	resource_info.type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
	resource_info.data.pAddressRange = &address_range;

	VkHostAddressRangeEXT target_address{};
	target_address.address = reinterpret_cast<void*>(slot->cpu_handle);
	target_address.size = slot->descriptor_size;

	if (vkWriteResourceDescriptorsEXT(
		*device_impl,
		1,                // resourceCount
		&resource_info,   // pResources
		&target_address   // pDescriptors
	) != VK_SUCCESS) {
		throw std::runtime_error("Error writing acceleration structure resource descriptor");
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