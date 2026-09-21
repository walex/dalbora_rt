#include "vk_device.hpp"
#include "vk_command_queue.hpp"

bool device_vk_check_heap_table_features(const VkPhysicalDevice physical_device) {
	VkPhysicalDeviceDescriptorHeapFeaturesEXT heapFeatures{};
	heapFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_HEAP_FEATURES_EXT;
	VkPhysicalDeviceFeatures2 deviceFeatures2{};
	deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	deviceFeatures2.pNext = &heapFeatures;
	vkGetPhysicalDeviceFeatures2(physical_device, &deviceFeatures2);
	return heapFeatures.descriptorHeap;
}

bool device_vk_check_ray_tracing_features(const VkPhysicalDevice physical_device) {
    
    VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures{};
    accelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;

    VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeatures{};
    rayTracingPipelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
    rayTracingPipelineFeatures.pNext = &accelerationStructureFeatures;

    VkPhysicalDeviceFeatures2 deviceFeatures2{};
    deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    deviceFeatures2.pNext = &rayTracingPipelineFeatures; // Encadenamos la lista de RT

    vkGetPhysicalDeviceFeatures2(physical_device, &deviceFeatures2);

    return accelerationStructureFeatures.accelerationStructure &&
        rayTracingPipelineFeatures.rayTracingPipeline;
}

bool device_vk_check_device_features(const VkPhysicalDevice physical_device, const size_t features,
    std::vector<const char*>& device_extensions) {

    device_extensions = {
        // swap chain extension
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
        VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME,
        VK_EXT_DESCRIPTOR_HEAP_EXTENSION_NAME
    };

    auto feats = features;
    if (feats & device_features_raytracing) {
        device_extensions.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
        device_extensions.push_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
        device_extensions.push_back(VK_KHR_RAY_QUERY_EXTENSION_NAME);
        device_extensions.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
        device_extensions.push_back(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    }

    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(device_extensions.begin(), device_extensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    if (requiredExtensions.empty() == false) {
        std::cout << "Extensions not supported:" << std::endl;
        for (const auto& missing : requiredExtensions) {
            std::cout << " - " << missing << std::endl;
        }
        return false;
    }

    if (device_vk_check_heap_table_features(physical_device) == false) {
		std::cout << "Descriptor heap feature not supported." << std::endl;
        return false;
    }

    if ((feats & device_features_raytracing)
        && device_vk_check_ray_tracing_features(physical_device) == false) {
		
        std::cout << "Ray tracing features not supported." << std::endl;
        return false;
    }
    feats ^= device_features_raytracing;
    return true;    
}

void device_vk_get_command_queue_family_indices(const VkPhysicalDevice physical_device, uint32_t& graphics_family_index,
    uint32_t& compute_family_index, uint32_t& copy_family_index) {

	graphics_family_index = -1;
	compute_family_index = -1;
	copy_family_index = -1;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queueFamilyCount, queueFamilies.data());

    for (uint32_t i = 0; i < queueFamilyCount; i++) {

        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            graphics_family_index = i;
        }
       
        if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
            compute_family_index = i;
        }

        if ((queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT) &&
            !(queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
            !(queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT)) {
            copy_family_index   = i;
            break;
        }

        if (graphics_family_index != -1 && compute_family_index != -1 
            && copy_family_index != -1) {
            break;
        }
    }

    if (copy_family_index == -1) {
        copy_family_index = graphics_family_index;
    }
}

VkDevice device_vk_create_logical_device(const VkInstance instance, const uint32_t graphics_queue_count,
    const uint32_t compute_queue_count, const unsigned long long device_features, 
    VkPhysicalDevice* physical_device_out = nullptr, uint32_t* graphics_queue_family_index_out = nullptr, 
    uint32_t* compute_queue_family_index_out = nullptr, uint32_t* copy_queue_family_index_out = nullptr) {

    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(instance, &device_count, nullptr);
    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(instance, &device_count, devices.data());
    
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    std::vector<const char*> device_extensions;
    uint32_t graphics_queue_family_index = -1;
    uint32_t compute_queue_family_index = -1;  
	uint32_t copy_family_index = -1;
	for (const auto& pdev : devices) {

        device_extensions.clear();
        if (device_vk_check_device_features(pdev, device_features, device_extensions) == true) {
            
            physical_device = pdev;
            break;
        }
	}

    if (physical_device == VK_NULL_HANDLE) {
        throw std::runtime_error("compatible physical device not found");
    }

    device_vk_get_command_queue_family_indices(physical_device, graphics_queue_family_index,
        compute_queue_family_index, copy_family_index);

    if (graphics_queue_family_index_out != nullptr && graphics_queue_family_index == -1) {
        throw std::runtime_error("compatible graphics queue family not found.");
    }
    if (compute_queue_family_index_out != nullptr && compute_queue_family_index == -1) {
        throw std::runtime_error("compatible compute queue family not found.");
    }
    if (copy_queue_family_index_out != nullptr && copy_family_index == -1) {
        throw std::runtime_error("compatible copy queue family not found.");
    }

	std::vector<VkDeviceQueueCreateInfo> v_queue_create_info(3);
    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo& queue_create_info = v_queue_create_info[0];
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = graphics_queue_family_index;
    queue_create_info.queueCount = graphics_queue_count;
    queue_create_info.pQueuePriorities = &queuePriority;

    queue_create_info = v_queue_create_info[1];
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = compute_queue_family_index;
    queue_create_info.queueCount = compute_queue_count;
    queue_create_info.pQueuePriorities = &queuePriority;

    VkPhysicalDeviceBufferDeviceAddressFeatures bdaFeatures{};
    bdaFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
    bdaFeatures.bufferDeviceAddress = VK_TRUE;

    VkPhysicalDeviceShaderUntypedPointersFeaturesKHR untypedPointersFeatures{};
    untypedPointersFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_UNTYPED_POINTERS_FEATURES_KHR;
    untypedPointersFeatures.pNext = &bdaFeatures;
    untypedPointersFeatures.shaderUntypedPointers = VK_TRUE;

    VkPhysicalDeviceDescriptorHeapFeaturesEXT heapFeatures{};
    heapFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_HEAP_FEATURES_EXT;
    heapFeatures.pNext = &untypedPointersFeatures;
    heapFeatures.descriptorHeap = VK_TRUE;

    VkDeviceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.pQueueCreateInfos = v_queue_create_info.data();
    create_info.queueCreateInfoCount = static_cast<uint32_t>(v_queue_create_info.size());
    create_info.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());
    create_info.ppEnabledExtensionNames = device_extensions.data();
    create_info.pNext = &heapFeatures;

	VkDevice device;
    if (vkCreateDevice(physical_device, &create_info, nullptr, &device) != VK_SUCCESS) {
        throw std::runtime_error("Error creating logical device!");
    }
	if (physical_device_out != nullptr)
        *physical_device_out = physical_device;
    if (graphics_queue_family_index_out != nullptr)
        *graphics_queue_family_index_out = graphics_queue_family_index;
    if (compute_queue_family_index_out != nullptr)
        *compute_queue_family_index_out = compute_queue_family_index;
	if (copy_queue_family_index_out != nullptr)
		*copy_queue_family_index_out = copy_family_index;
    return device;
}

RHI_DEVICE* vk_device_create(const RHI_DEVICE_DESC* const desc) {
	
    ASSERT_PTR(desc);
    ASSERT_PTR(desc->app_instance);
    
	VkPhysicalDevice physical_device = VK_NULL_HANDLE;
	uint32_t graphics_queue_family_index = -1;
	uint32_t compute_queue_family_index = -1;
    VkDevice device = device_vk_create_logical_device(static_cast<VkInstance>(desc->app_instance), desc->graphics_queue_count,
        desc->compute_queue_count, desc->features, 
        &physical_device, &graphics_queue_family_index,
        &compute_queue_family_index);
    ASSERT_PTR(device);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = graphics_queue_family_index;

    VkCommandPool graphics_queue_command_pool = VK_NULL_HANDLE;
    if (graphics_queue_family_index >= 0) {
        if (vkCreateCommandPool(device, &poolInfo, nullptr, &graphics_queue_command_pool) != VK_SUCCESS) {
            throw std::runtime_error("Error creating graphics command pool");
        }
        ASSERT_PTR(graphics_queue_command_pool);
    }

    poolInfo.queueFamilyIndex = compute_queue_family_index;
    VkCommandPool compute_queue_command_pool = VK_NULL_HANDLE;
    if (compute_queue_family_index >= 0) {
        if (vkCreateCommandPool(device, &poolInfo, nullptr, &compute_queue_command_pool) != VK_SUCCESS) {
            throw std::runtime_error("Error creating compute command pool");
        }
        ASSERT_PTR(compute_queue_command_pool);
    }

	VK_DEVICE* vk_device = new VK_DEVICE();
	vk_device->set_handle(device);
	vk_device->physical_device = physical_device;
    vk_device->graphics_queue_command_pool = graphics_queue_command_pool;
	vk_device->graphics_queue_family_index = graphics_queue_family_index;
    vk_device->compute_queue_command_pool = compute_queue_command_pool;
	vk_device->compute_queue_family_index = compute_queue_family_index;
    return vk_device;
}