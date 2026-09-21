#include "vk_device.hpp"
#include "vk_command_queue.hpp"

bool check_vk_heap_table_features(const VkPhysicalDevice physical_device) {
	VkPhysicalDeviceDescriptorHeapFeaturesEXT heapFeatures{};
	heapFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_HEAP_FEATURES_EXT;
	VkPhysicalDeviceFeatures2 deviceFeatures2{};
	deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	deviceFeatures2.pNext = &heapFeatures;
	vkGetPhysicalDeviceFeatures2(physical_device, &deviceFeatures2);
	return heapFeatures.descriptorHeap;
}

bool check_vk_ray_tracing_features(const VkPhysicalDevice physical_device) {
    
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

bool check_vk_device_features(const VkPhysicalDevice physical_device, const size_t features,
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

    if (check_vk_heap_table_features(physical_device) == false) {
		std::cout << "Descriptor heap feature not supported." << std::endl;
        return false;
    }

    if ((feats & device_features_raytracing)
        && check_vk_ray_tracing_features(physical_device) == false) {
		
        std::cout << "Ray tracing features not supported." << std::endl;
        return false;
    }
    feats ^= device_features_raytracing;
    return true;    
}

VkDevice create_vk_logical_device(const VkInstance instance, const uint32_t graphics_queue_count,
    const uint32_t compute_queue_count, const unsigned long long device_features, VkPhysicalDevice* physical_device_out) {

    ASSERT_PTR(physical_device_out);

    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(instance, &device_count, nullptr);
    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(instance, &device_count, devices.data());
    
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    std::vector<const char*> device_extensions;
    uint32_t graphics_family_index = -1;
    uint32_t compute_family_index = -1;  
	for (const auto& pdev : devices) {

        device_extensions.clear();
        if (check_vk_device_features(pdev, device_features, device_extensions) == true) {
            
            physical_device = pdev;
            break;
        }
	}

    if (physical_device == VK_NULL_HANDLE) {
        throw std::runtime_error("compatible physical device not found");
    }

    vk_command_queue_get_queue_family_indices(physical_device, graphics_family_index,
        compute_family_index);

    if (graphics_family_index == -1) {
        throw std::runtime_error("compatible graphics queue family not found.");
    }
    if (compute_family_index == -1) {
        throw std::runtime_error("compatible compute queue family not found.");
    }

	std::vector<VkDeviceQueueCreateInfo> v_queue_create_info(2);
    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo& queue_create_info = v_queue_create_info[0];
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = graphics_family_index;
    queue_create_info.queueCount = graphics_queue_count;
    queue_create_info.pQueuePriorities = &queuePriority;

    queue_create_info = v_queue_create_info[1];
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = compute_family_index;
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
    *physical_device_out = physical_device;
    return device;
}

RHI_DEVICE* vk_device_create(const RHI_DEVICE_DESC* const desc) {
	
    ASSERT_PTR(desc);
    ASSERT_PTR(desc->app_instance);
    
	VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    VkDevice device = create_vk_logical_device(static_cast<VkInstance>(desc->app_instance), desc->graphics_queue_count,
        desc->compute_queue_count, desc->features, &physical_device);
    ASSERT_PTR(device);
    ASSERT_PTR(physical_device);
	VK_DEVICE* vk_device = new VK_DEVICE();
	vk_device->set_handle(device);
	vk_device->physical_device = physical_device;

    return vk_device;
}