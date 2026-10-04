#include "vk_buffers.hpp"
#include "vk_heap.hpp"

static uint32_t find_memory_type_index(VkPhysicalDevice physical_device, uint32_t type_filter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties mem_properties;
	vkGetPhysicalDeviceMemoryProperties(physical_device, &mem_properties);

	for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++) {
		if ((type_filter & (1 << i)) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}
	ASSERT_EXPR(false, "Compatible memory type with Vulkan not found");
	return 0;
}

static RHI_VIEW* create_view_from_buffer_view_vk(const RHI_VIEW_DESC* const desc, const VkBufferViewCreateInfo& view_info,
    const RHI_MEMORY_DESCRIPTOR_SLOT* const slot) {

	ASSERT_PTR(desc);
    ASSERT_PTR(desc->device);
    ASSERT_PTR(slot);
    
    VkBufferView buffer_view = VK_NULL_HANDLE;
    vkCreateBufferView(*static_cast<VK_DEVICE*>(desc->device), &view_info, nullptr, &buffer_view);
    ASSERT_PTR(buffer_view);

	VK_VIEW* result = new VK_VIEW();
	ASSERT_PTR(result);
    result->set_handle(buffer_view);
	result->buffer = *static_cast<const VK_BUFFER*>(desc->buffer);
	result->type = desc->type;
	result->format = desc->format;
	result->mip_map_count = 1;
	result->memory_descriptor = desc->memory_descriptor;

	return result;
}

static RHI_VIEW* create_view_from_image_view_vk(const RHI_VIEW_DESC* const desc, const VkImageViewCreateInfo& view_info,
    const RHI_MEMORY_DESCRIPTOR_SLOT* const slot) {
	
    ASSERT_PTR(desc);
    ASSERT_PTR(desc->device);
    ASSERT_PTR(slot);

    VkImageView image_view = VK_NULL_HANDLE;
    vkCreateImageView(*static_cast<VK_DEVICE*>(desc->device), &view_info, nullptr, &image_view);
    ASSERT_PTR(image_view);
    
	VK_IMAGE_VIEW* result = new VK_IMAGE_VIEW();
	ASSERT_PTR(result);
	result->set_handle(image_view);
	result->buffer = *static_cast<const VK_BUFFER*>(desc->buffer);
	result->type = desc->type;
	result->format = desc->format;
	result->mip_map_count = desc->mip_maps_count;
	result->memory_descriptor = desc->memory_descriptor;

    vk_memory_resource_write_image_descriptor(*static_cast<VK_DEVICE*>(desc->device), slot, &view_info);

	return result;
}

static RHI_VIEW* create_view_from_agnostic_buffer_vk(const RHI_VIEW_DESC* const desc, const RHI_MEMORY_DESCRIPTOR_SLOT* const slot) {

	VkDescriptorBufferInfo buffer_info{};
	buffer_info.buffer = *static_cast<const VK_BUFFER*>(desc->buffer);
	buffer_info.offset = 0;
	buffer_info.range = desc->buffer->length;

	VK_AGNOSTIC_BUFFER_VIEW* result = new VK_AGNOSTIC_BUFFER_VIEW();
	result->buffer = make_observer_ptr<RHI_BUFFER>(desc->buffer);
	result->offset = buffer_info.offset;
	result->range = buffer_info.range;

	vk_memory_resource_write_constant_buffer_view_descriptor(*static_cast<VK_DEVICE*>(desc->device), slot, &buffer_info);
}

static RHI_VIEW* create_view_from_tlas_vk(const RHI_VIEW_DESC* const desc, const RHI_MEMORY_DESCRIPTOR_SLOT* const slot) {

	ASSERT_PTR(desc);
	ASSERT_PTR(desc->device);
	ASSERT_PTR(slot);

	VkAccelerationStructureKHR bvh = *static_cast<const VK_BUFFER*>(desc->buffer);

	VK_AGNOSTIC_BUFFER_VIEW* result = new VK_AGNOSTIC_BUFFER_VIEW();
	ASSERT_PTR(result);
	result->buffer = make_observer_ptr<RHI_BUFFER>(desc->buffer);
	result->offset = 0;
	result->range = desc->buffer->length;

	vk_memory_resource_write_acceleration_structure_descriptor(*static_cast<VK_DEVICE*>(desc->device), slot, &bvh);

	return result;
}

template <typename T>
static T* buffers_create_2d_vk(const RHI_BUFFER_2D_DESC* const desc)
{
	ASSERT_PTR(desc);
	ASSERT_PTR(desc->device);

	VkDevice device = *static_cast<VK_DEVICE*>(desc->device);
	ASSERT_PTR(device);

	VkPhysicalDevice physical_device = static_cast<VK_DEVICE*>(desc->device)->physical_device;
	ASSERT_PTR(physical_device);

	buffer_type buffer_type = desc->type;

	// Si es BVH (Ray Tracing), en Vulkan es un VkBuffer, no una VkImage
	if (buffer_type == buffer_type_bvh)
	{
		VkBufferCreateInfo buffer_info = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		buffer_info.size = desc->length; // O width * stride
		buffer_info.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR |
			VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
		buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkBuffer vk_buffer = VK_NULL_HANDLE;
		VkResult res = vkCreateBuffer(device, &buffer_info, nullptr, &vk_buffer);
		ASSERT_VK_RESULT(res);

		// Requerimientos de memoria y asignación
		VkMemoryRequirements mem_reqs;
		vkGetBufferMemoryRequirements(device, vk_buffer, &mem_reqs);

		VkMemoryAllocateFlagsInfo alloc_flags_info = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO };
		alloc_flags_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;

		VkMemoryAllocateInfo alloc_info = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
		alloc_info.pNext = &alloc_flags_info;
		alloc_info.allocationSize = mem_reqs.size;
		alloc_info.memoryTypeIndex = find_memory_type_index(physical_device, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VkDeviceMemory vk_memory = VK_NULL_HANDLE;
		res = vkAllocateMemory(device, &alloc_info, nullptr, &vk_memory);
		ASSERT_VK_RESULT(res);

		res = vkBindBufferMemory(device, vk_buffer, vk_memory, 0);
		ASSERT_VK_RESULT(res);

		T* buffer_impl = new T();
		ASSERT_PTR(buffer_impl);
		buffer_impl->length = desc->length;
		buffer_impl->format = desc->format;
		buffer_impl->stride = desc->stride;
		buffer_impl->type = desc->type;
		buffer_impl->set_handle(vk_buffer);
		// TODO:
		// if is_external_memory-pool == true
		//	make_observer_ptr<VK_MEMORY_POOL>(vk_memory);
		// else
			buffer_impl->memory_pool = make_releseable_observer_ptr<VK_MEMORY_POOL>(new VK_MEMORY_POOL());
		buffer_impl->memory_pool->set_handle(vk_memory);
		return buffer_impl;
	}

	// Configuración de Flags de Uso para VkImage
	VkImageUsageFlags usage_flags = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

	if (buffer_type == buffer_type_depth_stencil) {
		usage_flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		buffer_type = buffer_type_image_2d;
	}

	if ((desc->flags & resource_flags_shader_read_write) == resource_flags_shader_read_write) {
		usage_flags |= VK_IMAGE_USAGE_STORAGE_BIT;
	}

	if (desc->is_render_target == true) {
		usage_flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	}

	// Creación de la VkImage
	VkImageCreateInfo image_info = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
	image_info.imageType = VK_IMAGE_TYPE_2D;
	image_info.format = vk_resource_format_type[desc->format];
	image_info.extent.width = static_cast<uint32_t>(desc->width);
	image_info.extent.height = static_cast<uint32_t>(desc->height);
	image_info.extent.depth = 1;
	image_info.mipLevels = static_cast<uint32_t>(desc->mips);
	image_info.arrayLayers = 1;
	image_info.samples = VK_SAMPLE_COUNT_1_BIT;
	image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	image_info.usage = usage_flags;
	image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	VkImage vk_image = VK_NULL_HANDLE;
	VkResult res = vkCreateImage(device, &image_info, nullptr, &vk_image);
	ASSERT_VK_RESULT(res);

	// Reservar memoria física para la imagen
	VkMemoryRequirements mem_reqs;
	vkGetImageMemoryRequirements(device, vk_image, &mem_reqs);

	VkMemoryPropertyFlags memory_properties = vk_heap_type[desc->memory_type]; // e.g., VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT

	VkMemoryAllocateInfo alloc_info = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
	alloc_info.allocationSize = mem_reqs.size;
	alloc_info.memoryTypeIndex = find_memory_type_index(physical_device, mem_reqs.memoryTypeBits, memory_properties);

	VkDeviceMemory vk_memory = VK_NULL_HANDLE;
	res = vkAllocateMemory(device, &alloc_info, nullptr, &vk_memory);
	ASSERT_VK_RESULT(res);

	res = vkBindImageMemory(device, vk_image, vk_memory, 0);
	ASSERT_VK_RESULT(res);

	T* buffer_impl = new T();
	ASSERT_PTR(buffer_impl);
	buffer_impl->length = desc->length;
	buffer_impl->format = desc->format;
	buffer_impl->stride = desc->stride;
	buffer_impl->type = desc->type;
	buffer_impl->set_handle(vk_image);
	// TODO: 
	// if is_external_memory-pool == true
	//	make_observer_ptr<VK_MEMORY_POOL>(vk_memory);
	// else
		buffer_impl->memory_pool = make_releseable_observer_ptr<VK_MEMORY_POOL>(new VK_MEMORY_POOL());
	buffer_impl->memory_pool->set_handle(vk_memory);
	return buffer_impl;
}

template <typename T>
static T* buffers_create_vk(const RHI_BUFFER_DESC* const desc) {

	ASSERT_PTR(desc);
	ASSERT_PTR(desc->device);

	RHI_BUFFER_2D_DESC desc_2d;
	desc_2d.device = desc->device;
	desc_2d.length = desc->length;
	desc_2d.mips = desc->mips;
	desc_2d.memory_type = desc->memory_type;
	desc_2d.format = desc->format;
	desc_2d.type = desc->type;
	desc_2d.width = desc->length;
	desc_2d.height = 1;
	desc_2d.stride = desc->stride;
	return buffers_create_2d_vk<T>(&desc_2d);
}

static VkImageView buffers_create_dsv_from_handle_vk(
    const VK_DEVICE* const device_impl,
    const RHI_BUFFER* const buffer,
    const resource_format format,
    const RHI_MEMORY_DESCRIPTOR_SLOT* const slot) {

	ASSERT_PTR(device_impl);
    ASSERT_PTR(slot);
    ASSERT_PTR(buffer);

    VkImageViewCreateInfo view_info{};
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.image = *static_cast<const VK_TEXTURE_2D*>(buffer);
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = vk_resource_format_type[format];
    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

    // Si incluye el stencil en el formato, añadimos el flag de stencil
    if (format == resource_format_d24_norm_s8_uint || format == resource_format_d32_float_s8_uint) {
        view_info.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;


    VkImageView image_view = VK_NULL_HANDLE;
    vkCreateImageView(*device_impl, &view_info, nullptr, &image_view);
    ASSERT_PTR(image_view);

    vk_memory_resource_write_image_descriptor(device_impl, slot, &view_info);

	return image_view;
}

static RHI_VIEW* buffers_create_dsv_vk(const RHI_VIEW_DESC* const desc) {
    ASSERT_PTR(desc);
    ASSERT_PTR(desc->device);
    ASSERT_PTR(desc->buffer);
    ASSERT_PTR(desc->memory_descriptor);

    VK_IMAGE_VIEW* result = new VK_IMAGE_VIEW();
    ASSERT_PTR(result);

    VkImageView image_view = buffers_create_dsv_from_handle_vk(static_cast<VK_DEVICE*>(desc->device), desc->buffer,
        desc->format, desc->memory_descriptor);
    
	result->set_handle(image_view);
    result->buffer = make_observer_ptr<RHI_BUFFER>(desc->buffer);
    result->type = desc->type;
    result->format = desc->format;
    result->mip_map_count = 1;
    result->memory_descriptor = desc->memory_descriptor;
    return result;
}

// --- RTV (Render Target View) ---

static VkImageView buffers_create_rtv_from_handle_vk(
    const VK_DEVICE* const device_impl,
    const RHI_BUFFER* const buffer,
    const resource_format format,
    const RHI_MEMORY_DESCRIPTOR_SLOT* const slot) {

    VkImageViewCreateInfo view_info{};
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.image = *static_cast<const VK_TEXTURE_2D*>(buffer);
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = vk_resource_format_type[format];
    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;

    VkImageView image_view = VK_NULL_HANDLE;
    vkCreateImageView(*device_impl, &view_info, nullptr, &image_view);
	ASSERT_PTR(image_view);

    vk_memory_resource_write_image_descriptor(device_impl, slot, &view_info);

    return image_view;
}

static RHI_VIEW* buffers_create_rtv_vk(const RHI_VIEW_DESC* const desc) {
    ASSERT_PTR(desc);
    ASSERT_PTR(desc->device);
    ASSERT_PTR(desc->buffer);
    ASSERT_PTR(desc->memory_descriptor);

    VK_IMAGE_VIEW* result = new VK_IMAGE_VIEW();
    ASSERT_PTR(result);

    VkImageView image_view = buffers_create_rtv_from_handle_vk(static_cast<VK_DEVICE*>(desc->device), desc->buffer, 
        desc->format, desc->memory_descriptor);

    result->set_handle(image_view);
    result->buffer = make_observer_ptr<RHI_BUFFER>(desc->buffer);
    result->type = desc->type;
    result->format = desc->format;
    result->mip_map_count = 1;
    result->memory_descriptor = desc->memory_descriptor;
    return result;
}

// --- CBV / SRV / UAV ---

static RHI_VIEW* buffers_create_cbv_srv_uav_from_handle_vk(
    const VK_DEVICE* const device_impl,
    const RHI_BUFFER* const buffer,
    const shader_view_type type,
    const size_t buffer_length,
    const size_t buffer_stride,
    const resource_format format,
    const size_t mip_maps_count,
    const RHI_MEMORY_DESCRIPTOR_SLOT* const slot) {

    RHI_VIEW* result = nullptr;

    // 1. CONSTANT BUFFER (CBV) -> Uniform Buffer Descriptor
    if (type == shader_view_type_constant_buffer) {
		RHI_VIEW_DESC desc;
		desc.type = type;
		desc.format = format;
		desc.mip_maps_count = mip_maps_count;
		desc.device = const_cast<VK_DEVICE*>(device_impl);
		desc.memory_descriptor = slot;
		result = create_view_from_agnostic_buffer_vk(&desc, slot);
    }
    // 2. READ-ONLY BUFFER (SRV) -> Structured Storage Buffer o Uniform Texel Buffer
    else if (type == shader_view_type_read_only_buffer) {
        if (format != resource_format_none) {
            // Buffer formateado (Typed / Texel Buffer)
            VkBufferViewCreateInfo view_info{};
            view_info.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
            view_info.buffer = *static_cast<const VK_BUFFER*>(buffer);
            view_info.format = vk_resource_format_type[format];
            view_info.offset = 0;
            view_info.range = buffer_length;

			RHI_VIEW_DESC desc;
			desc.type = type;
			desc.format = format;
			desc.mip_maps_count = mip_maps_count;
			desc.device = const_cast<VK_DEVICE*>(device_impl);
			desc.memory_descriptor = slot;
			desc.buffer = const_cast<RHI_BUFFER*>(buffer);
			result = create_view_from_buffer_view_vk(&desc, view_info, slot);            
        }
        else {
			RHI_VIEW_DESC desc;
			desc.type = type;
			desc.format = format;
			desc.mip_maps_count = mip_maps_count;
			desc.device = const_cast<VK_DEVICE*>(device_impl);
			desc.memory_descriptor = slot;
			result = create_view_from_agnostic_buffer_vk(&desc, slot);
        }
    }
    // 3. READ-WRITE BUFFER (UAV) -> RW Structured Storage Buffer o Storage Texel Buffer
    else if (type == shader_view_type_rw_buffer) {
        if (format != resource_format_none) {
            // RW Typed Texel Buffer
            VkBufferViewCreateInfo view_info{};
            view_info.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
            view_info.buffer = *static_cast<const VK_BUFFER*>(buffer);
            view_info.format = vk_resource_format_type[format];
            view_info.offset = 0;
            view_info.range = buffer_length;

			RHI_VIEW_DESC desc;
			desc.type = type;
			desc.format = format;
			desc.mip_maps_count = mip_maps_count;
			desc.device = const_cast<VK_DEVICE*>(device_impl);
			desc.memory_descriptor = slot;
			desc.buffer = const_cast<RHI_BUFFER*>(buffer);

            result = create_view_from_buffer_view_vk(&desc, view_info, slot);
        }
        else {
			RHI_VIEW_DESC desc;
			desc.type = type;
			desc.format = format;
			desc.mip_maps_count = mip_maps_count;
			desc.device = const_cast<VK_DEVICE*>(device_impl);
			desc.memory_descriptor = slot;
			result = create_view_from_agnostic_buffer_vk(&desc, slot);
        }
    }
    // 4. READ-ONLY TEXTURE (SRV Textura 2D) -> Sampled Image
    else if (type == shader_view_type_read_only_texture_buffer) {
        VkImageViewCreateInfo view_info{};
        view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_info.image = *static_cast<const VK_TEXTURE_2D*>(buffer);
        view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view_info.format = vk_resource_format_type[format];
        view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        view_info.subresourceRange.baseMipLevel = 0;
        view_info.subresourceRange.levelCount = static_cast<uint32_t>(mip_maps_count);
        view_info.subresourceRange.baseArrayLayer = 0;
        view_info.subresourceRange.layerCount = 1;

		RHI_VIEW_DESC desc;
		desc.type = type;
		desc.format = format;
		desc.mip_maps_count = mip_maps_count;
		desc.device = const_cast<VK_DEVICE*>(device_impl);
		desc.memory_descriptor = slot;
		desc.buffer = const_cast<RHI_BUFFER*>(buffer);
        result = create_view_from_image_view_vk(&desc, view_info, slot);
    }
    // 5. READ-WRITE TEXTURE (UAV Textura 2D) -> Storage Image
    else if (type == shader_view_type_rw_texture_buffer) {
        VkImageViewCreateInfo view_info{};
        view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_info.image = *static_cast<const VK_TEXTURE_2D*>(buffer);
        view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view_info.format = vk_resource_format_type[format];
        view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        view_info.subresourceRange.baseMipLevel = 0;
        view_info.subresourceRange.levelCount = 1; // Generalmente las UAVs de textura acceden a 1 mip específico
        view_info.subresourceRange.baseArrayLayer = 0;
        view_info.subresourceRange.layerCount = 1;

		RHI_VIEW_DESC desc;
		desc.type = type;
		desc.format = format;
		desc.mip_maps_count = mip_maps_count;
		desc.device = const_cast<VK_DEVICE*>(device_impl);
		desc.memory_descriptor = slot;
		desc.buffer = const_cast<RHI_BUFFER*>(buffer);
        result = create_view_from_image_view_vk(&desc, view_info, slot);
    }
    // 6. RAY TRACING BVH (SRV TLAS) -> Acceleration Structure
    else if (type == shader_view_type_bvh_buffer) {
		RHI_VIEW_DESC desc;
		desc.type = type;
		desc.format = format;
		desc.mip_maps_count = mip_maps_count;
		desc.device = const_cast<VK_DEVICE*>(device_impl);
		desc.memory_descriptor = slot;
		desc.buffer = const_cast<RHI_BUFFER*>(buffer);
		result = create_view_from_tlas_vk(&desc, slot);
    }

    ASSERT_PTR(result);
    return result;
}

static RHI_VIEW* buffers_create_cbv_srv_uav_vk(const RHI_VIEW_DESC* const desc) {
    ASSERT_PTR(desc);
    ASSERT_PTR(desc->device);
    ASSERT_PTR(desc->buffer);
    ASSERT_PTR(desc->memory_descriptor);

    return buffers_create_cbv_srv_uav_from_handle_vk(
        static_cast<VK_DEVICE*>(desc->device),
        desc->buffer,
        desc->type,
        desc->buffer->length,
        desc->buffer->stride,
        desc->format,
        desc->mip_maps_count,
        desc->memory_descriptor
    );
}

RHI_BUFFER* vk_buffers_create_linear(const RHI_BUFFER_DESC* const desc) {
	return buffers_create_vk<VK_BUFFER>(desc);
}

RHI_BUFFER* vk_buffers_create_2d(const RHI_BUFFER_2D_DESC* const desc) {
	return buffers_create_2d_vk<VK_BUFFER>(desc);
}

RHI_BUFFER* vk_buffers_create_constant(const RHI_BUFFER_DESC* const desc) {
	return buffers_create_vk<VK_BUFFER>(desc);
}

RHI_BUFFER* vk_buffers_create_indices(const RHI_INDEX_BUFFER_DESC* const desc) {
	ASSERT_PTR(desc);
	ASSERT_PTR(desc->device);

	RHI_BUFFER_DESC ib_desc;
	ib_desc.device = desc->device;
	ib_desc.length = desc->count * desc->stride;
	ib_desc.memory_type = desc->memory_type;
	ib_desc.type = desc->type;
	ib_desc.format = desc->format;
	ib_desc.mips = 1;
	ib_desc.stride = desc->stride;
	return buffers_create_vk<VK_BUFFER>(&ib_desc);
}

RHI_BUFFER* vk_buffers_create_vertices(const RHI_VERTEX_BUFFER_DESC* const desc) {
	ASSERT_PTR(desc);
	ASSERT_PTR(desc->device);

	// overwrite desc to match must have index buffer requeriments
	RHI_BUFFER_DESC vb_desc;
	vb_desc.device = desc->device;
	vb_desc.length = desc->count * desc->stride;
	vb_desc.memory_type = desc->memory_type;
	vb_desc.type = desc->type;
	vb_desc.format = desc->format;
	vb_desc.mips = 1;
	vb_desc.stride = desc->stride;
	return buffers_create_vk<VK_BUFFER>(&vb_desc);
}

RHI_BUFFER* vk_buffers_create_depth(const RHI_BUFFER_2D_DESC* const desc) {
	ASSERT_PTR(desc);
	ASSERT_EXPR(desc->format >= resource_format_d32_float_s8_uint
		&& desc->format < resource_format_d16_norm);

	// overwrite desc to match must have depth buffer requeriments
	RHI_BUFFER_2D_DESC db_desc_mutable = *desc;
	db_desc_mutable.memory_type = buffer_memory_type_default;
	db_desc_mutable.type = buffer_type_depth_stencil;
	db_desc_mutable.mips = 1;
	db_desc_mutable.width = desc->width;
	db_desc_mutable.height = desc->height;
	db_desc_mutable.format = desc->format;
	return buffers_create_2d_vk<VK_TEXTURE_2D>(&db_desc_mutable);
}

void vk_buffers_copy_buffer_region(RHI_COMMAND_BUFFER* const command_buffer, const RHI_BUFFER* const src_buffer,
	size_t offset_src, RHI_BUFFER* const dest_buffer,
	size_t offset_dest, size_t length) {

}

void vk_buffers_copy_buffer(RHI_COMMAND_BUFFER* const command_buffer, const RHI_BUFFER* const src_buffer,
	RHI_BUFFER* const dest_buffer) {

}

void vk_buffers_gpu_upload_region(RHI_COMMAND_BUFFER* const command_buffer, const RHI_BUFFER* const src_buffer,
	RHI_BUFFER* const dest_buffer, const size_t offset_src,
	const size_t offset_dest, const size_t length) {

}

void vk_buffers_gpu_upload(RHI_COMMAND_BUFFER* const command_buffer, const RHI_BUFFER* const src_buffer,
	RHI_BUFFER* const gpu_buffer) {

}

void vk_buffers_gpu_download_region(RHI_COMMAND_BUFFER* const command_buffer, const RHI_BUFFER* const src_buffer,
	RHI_BUFFER* const gpu_buffer, const size_t offset_src,
	const size_t offset_dest, const size_t length) {

}

void vk_buffers_gpu_download(RHI_COMMAND_BUFFER* const command_buffer, const RHI_BUFFER* const src_buffer,
	RHI_BUFFER* const gpu_buffer) {

}

RHI_VOID_PTR vk_buffers_map_open(RHI_BUFFER* const buffer, const size_t offset,
	const size_t length) {
	return nullptr;
}

void vk_buffers_map_close(RHI_BUFFER* const buffer, const size_t offset,
	const size_t length) {
    
}

void vk_buffers_map_write(RHI_BUFFER* const buffer, const RHI_VOID_PTR data,
	const size_t offset, const size_t length) {

}

void vk_buffers_map_read(RHI_BUFFER* const buffer, RHI_VOID_PTR* const data,
	const size_t offset, const size_t length) {

}

RHI_VIEW* vk_buffers_create_view(const RHI_VIEW_DESC* const desc) {
    return nullptr;
}

void vk_buffers_update_view(const RHI_DEVICE* const device,
	RHI_VIEW* const view,
	const RHI_BUFFER* const buffer) {

}

//
//
//void vk_buffers_create_view(const RHI_VIEW_DESC* const view_desc) {
//
//	ASSERT_PTR(view_desc);
//}
//
//// 1. Datos de origen en CPU
//std::vector<Vertex> vertices = { ... };
//std::vector<uint16_t> indices = { ... };
//
//VkDeviceSize vertexSize = sizeof(vertices[0]) * vertices.size();
//VkDeviceSize indexSize = sizeof(indices[0]) * indices.size();
//
//// 2. Crear el STAGING BUFFER (Visible para CPU)
//VkBuffer stagingBuffer;
//VkDeviceMemory stagingBufferMemory;
//crearBuffer(vertexSize + indexSize,
//    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
//    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
//    stagingBuffer, stagingBufferMemory);
//
//// 3. MAPPING: Copiar datos al Staging Buffer
//void* data;
//vkMapMemory(device, stagingBufferMemory, 0, vertexSize + indexSize, 0, &data);
//memcpy(data, vertices.data(), (size_t)vertexSize);
//memcpy((static_cast<char*>(data) + vertexSize), indices.data(), (size_t)indexSize);
//vkUnmapMemory(device, stagingBufferMemory);
//
//// 4. Crear los búferes FINALES en GPU (Device Local)
//VkBuffer vertexBuffer;
//VkDeviceMemory vertexBufferMemory;
//crearBuffer(vertexSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
//    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);
//
//VkBuffer indexBuffer;
//VkDeviceMemory indexBufferMemory;
//crearBuffer(indexSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
//    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);
//
//// 5. Transferencia mediante COMMAND QUEUE
//VkCommandBuffer commandBuffer = comenzarCommandosInmediatos();
//
//VkBufferCopy copyRegionVertices{};
//copyRegionVertices.srcOffset = 0;
//copyRegionVertices.dstOffset = 0;
//copyRegionVertices.size = vertexSize;
//vkCmdCopyBuffer(commandBuffer, stagingBuffer, vertexBuffer, 1, &copyRegionVertices);
//
//VkBufferCopy copyRegionIndices{};
//copyRegionIndices.srcOffset = vertexSize; // Desplazamiento en el staging buffer
//copyRegionIndices.dstOffset = 0;
//copyRegionIndices.size = indexSize;
//vkCmdCopyBuffer(commandBuffer, stagingBuffer, indexBuffer, 1, &copyRegionIndices);
//
//terminarYEnviarCommandos(commandBuffer, queue);
//
//// 6. Limpieza del búfer intermedio
//vkDestroyBuffer(device, stagingBuffer, nullptr);
//vkFreeMemory(device, stagingBufferMemory, nullptr);
//
//
//
//--------------------------------------------------------------------------
//
//// 1. Tamaños y Offsets
//VkDeviceSize vertexSize = sizeof(vertices[0]) * vertices.size();
//VkDeviceSize indexSize = sizeof(indices[0]) * indices.size();
//VkDeviceSize totalSize = vertexSize + indexSize;
//
//VkDeviceSize indexOffset = vertexSize; // El bloque de índices empieza justo después de los vértices
//
//// 2. Crear STAGING BUFFER único
//VkBuffer stagingBuffer;
//VkDeviceMemory stagingBufferMemory;
//crearBuffer(totalSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
//    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
//    stagingBuffer, stagingBufferMemory);
//
//// 3. MAPPING idéntico al Ejemplo 1
//void* data;
//vkMapMemory(device, stagingBufferMemory, 0, totalSize, 0, &data);
//memcpy(data, vertices.data(), (size_t)vertexSize);
//memcpy(static_cast<char*>(data) + indexOffset, indices.data(), (size_t)indexSize);
//vkUnmapMemory(device, stagingBufferMemory);
//
//// 4. Crear un ÚNICO BÚFER FINAL con múltiples banderas de uso (USAGE)
//VkBuffer geometryBuffer;
//VkDeviceMemory geometryBufferMemory;
//crearBuffer(totalSize,
//    VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
//    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, geometryBuffer, geometryBufferMemory);
//
//// 5. Transferencia en lote (Batch Copy) a la COMMAND QUEUE
//VkCommandBuffer commandBuffer = comenzarCommandosInmediatos();
//
//// Copiamos todo el bloque de memoria de golpe usando regiones mapeadas uniformemente
//VkBufferCopy copyRegion{};
//copyRegion.srcOffset = 0;
//copyRegion.dstOffset = 0;
//copyRegion.size = totalSize;
//
//vkCmdCopyBuffer(commandBuffer, stagingBuffer, geometryBuffer, 1, &copyRegion);
//
//terminarYEnviarCommandos(commandBuffer, queue);
//
//// Destrucción del staging
//vkDestroyBuffer(device, stagingBuffer, nullptr);
//vkFreeMemory(device, stagingBufferMemory, nullptr);
//
//
//------------------------------------
//
//uso
//
//VkDeviceSize offsets[] = { 0 };
//vkCmdBindVertexBuffers(commandBuffer, 0, 1, &geometryBuffer, offsets);
//vkCmdBindIndexBuffer(commandBuffer, geometryBuffer, indexOffset, VK_INDEX_TYPE_UINT16);
//
//
//VkCommandBuffer comenzarCommandosInmediatos() {
//    VkCommandBufferAllocateInfo allocInfo{};
//    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
//    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
//    allocInfo.commandPool = commandPool; // Tu VkCommandPool existente
//    allocInfo.commandBufferCount = 1;
//
//    VkCommandBuffer commandBuffer;
//    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);
//
//    VkCommandBufferBeginInfo beginInfo{};
//    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // Se destruye tras usarse
//
//    vkBeginCommandBuffer(commandBuffer, &beginInfo);
//    return commandBuffer;
//}
//
//void terminarYEnviarCommandos(VkCommandBuffer commandBuffer, VkQueue queue) {
//    vkEndCommandBuffer(commandBuffer);
//
//    VkSubmitInfo submitInfo{};
//    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
//    submitInfo.commandBufferCount = 1;
//    submitInfo.pCommandBuffers = &commandBuffer;
//
//    // Enviar a la cola de comandos y esperar a que la GPU termine de copiar
//    vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
//    vkQueueWaitIdle(queue);
//
//    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
//}