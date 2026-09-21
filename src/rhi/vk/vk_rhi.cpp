#include "vk_rhi.hpp"
#include "vk_swap_chain.hpp"
#include "vk_device.hpp"
#include "vk_heap.hpp"

VkInstance create_vk_instance() {

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_4;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    // Aquí se añadirían las extensiones globales requeridas por la ventana
    createInfo.enabledExtensionCount = 0;
    createInfo.ppEnabledExtensionNames = nullptr;

    VkInstance instance;
    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
        throw std::runtime_error("¡Error al crear la instancia de Vulkan!");
    }
    return instance;
}

static VkInstance app_instance = nullptr;

void vk_rhi_init() {

    if (volkInitialize() != VK_SUCCESS) {
        throw std::runtime_error("Error: cannot initialize volk.");
    }

    app_instance = create_vk_instance();

    volkLoadInstance(app_instance);

    // device
    rhi_create_device = &vk_device_create;

    // heap
    rhi_memory_resource_create = &vk_memory_resource_create;
    rhi_memory_resource_get_descriptor = &vk_memory_resource_get_descriptor;
}

void vk_rhi_end() {

    if (app_instance != nullptr) {
        vkDestroyInstance(static_cast<VkInstance>(app_instance), nullptr);
        app_instance = nullptr;
    }
}

RHI_APP_INSTANCE vk_rhi_get_app_instance() {
	return app_instance;
}