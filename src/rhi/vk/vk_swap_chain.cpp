#include "vk_swap_chain.hpp"

//
//void createSwapChain(uint32_t width, uint32_t height) {
//    VkSwapchainCreateInfoKHR createInfo{};
//    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
//    createInfo.surface = surface; // Superficie vinculada a la ventana de Windows/Linux
//
//    // Configuración de resolución y formato
//    createInfo.minImageCount = 2; // Doble buffer (pueden ser 3 para Triple buffer)
//    createInfo.imageFormat = VK_FORMAT_B8G8R8A8_SRGB;
//    createInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
//    createInfo.imageExtent = VkExtent2D{ width, height };
//    createInfo.imageArrayLayers = 1;
//    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
//
//    // Compartir imágenes entre colas (Modo exclusivo si solo se usa una cola de gráficos)
//    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
//    createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
//    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
//    createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR; // Equivalente a V-Sync activado
//    createInfo.clipped = VK_TRUE;
//    createInfo.oldSwapchain = VK_NULL_HANDLE;
//
//    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
//        throw std::runtime_error("¡Error al crear la Swap Chain!");
//    }
//
//    // Obtener los controladores de las imágenes creadas internamente por la Swap Chain
//    uint32_t imageCount;
//    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
//    swapChainImages.resize(imageCount);
//    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());
//
//    // Crear un Image View para cada imagen de la Swap Chain
//    swapChainImageViews.resize(swapChainImages.size());
//    for (size_t i = 0; i < swapChainImages.size(); i++) {
//        VkImageViewCreateInfo viewInfo{};
//        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
//        viewInfo.image = swapChainImages[i];
//        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
//        viewInfo.format = VK_FORMAT_B8G8R8A8_SRGB;
//        viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
//        viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
//        viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
//        viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
//        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//        viewInfo.subresourceRange.baseMipLevel = 0;
//        viewInfo.subresourceRange.levelCount = 1;
//        viewInfo.subresourceRange.baseArrayLayer = 0;
//        viewInfo.subresourceRange.layerCount = 1;
//
//        if (vkCreateImageView(device, &viewInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
//            throw std::runtime_error("¡Error al crear los Image Views!");
//        }
//    }
//}
//
//// 4. Limpieza de memoria (Destrucción en orden inverso)
//void cleanup() {
//    for (auto imageView : swapChainImageViews) {
//        vkDestroyImageView(device, imageView, nullptr);
//    }
//    vkDestroySwapchainKHR(device, swapChain, nullptr);
//    vkDestroyDevice(device, nullptr);
//    // vkDestroySurfaceKHR(instance, surface, nullptr); // Se destruye aquí si pertenece a la app
//    vkDestroyInstance(instance, nullptr);
//}

typedef VkSurfaceKHR(*create_surface_func_ptr)(const RHI_WINDOW* const window, const RHI_VOID_PTR* const param);

#ifdef WINDOWS_PLATFORM
VkSurfaceKHR create_surface_win32(const RHI_WINDOW* const window, const RHI_VOID_PTR* const param) {

    VkInstance instance = static_cast<VkInstance>(rhi_get_app_instance());

    VkWin32SurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.hinstance = GetModuleHandle(NULL);
    createInfo.hwnd = static_cast<HWND>(window->handle);           // Manejador de tu ventana Win32

    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkResult result = vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &surface);

    if (result != VK_SUCCESS) {
        // Manejar el error de creación
        return VK_NULL_HANDLE;
    }

    return surface;
}

#endif

#ifdef LINUX_PLATFORM
VkSurfaceKHR create_surface_linux(const RHI_WINDOW* const window, const RHI_VOID_PTR* const param) {

    VkInstance instance = static_cast<VkInstance>(rhi_get_app_instance());
    wl_surface* surface_wl = static_cast<wl_surface*>(param);

    VkWaylandSurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.display = static_cast<wl_display*>(window->handle);
    createInfo.surface = surface_wl;   

    VkSurfaceKHR surface = VK_NULL_HANDLE;
    if (vkCreateWaylandSurfaceKHR(instance, &createInfo, nullptr, &surface) != VK_SUCCESS) {
        return VK_NULL_HANDLE;
    }
    return surface;
}
#endif

#ifdef ANDROID_PLATFORM
VkSurfaceKHR create_surface_android(const RHI_WINDOW* const window, const RHI_VOID_PTR* const param) {

    VkInstance instance = static_cast<VkInstance>(rhi_get_app_instance());

    VkAndroidSurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.window = static_cast<ANativeWindow*>(window->handle);;

    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkResult result = vkCreateAndroidSurfaceKHR(instance, &createInfo, nullptr, &surface);

    if (result != VK_SUCCESS) {
        // Manejar error de creación
        return VK_NULL_HANDLE;
    }

    return surface;
}
#endif

RHI_SWAP_CHAIN* vk_swap_chain_create(const RHI_SWAP_CHAIN_DESC* const desc) {

    ASSERT_PTR(desc);
    ASSERT_PTR(desc->device);
    ASSERT_PTR(desc->command_queue);
    ASSERT_PTR(desc->window);
    ASSERT_PTR(desc->window->handle);

#ifdef WINDOWS_PLATFORM
     static create_surface_func_ptr create_surface_ptr = create_surface_win32;
#endif

#ifdef LINUX_PLATFORM
     static create_surface_func_ptr create_surface_ptr = create_surface_linux;
#endif

#ifdef ANDROID_PLATFORM
     static create_surface_func_ptr create_surface_ptr = create_surface_android;
#endif

    VkSurfaceKHR surface = create_surface_ptr(desc->window, nullptr);
    ASSERT_PTR(surface);

	VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface; // Superficie vinculada a la ventana de Windows/Linux

    createInfo.minImageCount = desc->buffer_count;
    createInfo.imageFormat = (desc->color_format != resource_format_none)
        ? vk_resource_format_type[desc->color_format]
        : VK_FORMAT_B8G8R8A8_SRGB;;
    createInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    createInfo.imageExtent = VkExtent2D{ static_cast<uint32_t>(desc->width), static_cast<uint32_t>(desc->height)};
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = desc->enable_vsync 
        ? VK_PRESENT_MODE_FIFO_KHR 
        : VK_PRESENT_MODE_IMMEDIATE_KHR;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    VkSwapchainKHR swap_chain;
    if (vkCreateSwapchainKHR(*static_cast<VK_DEVICE*>(desc->device), &createInfo, 
        nullptr, &swap_chain) != VK_SUCCESS) {
        throw std::runtime_error("¡Error al crear la Swap Chain!");
    }
    ASSERT_PTR(swap_chain);

    VK_SWAP_CHAIN* result = new VK_SWAP_CHAIN();
    ASSERT_PTR(result);
    result->set_handle(swap_chain);
    result->parent_device = static_cast<VK_DEVICE*>(desc->device);
	result->native_surface = surface;
    return result;
}

void vk_swap_chain_present(const RHI_SWAP_CHAIN* const swap_chain) {

}

uint32_t vk_swap_chain_get_current_buffer_id(const RHI_SWAP_CHAIN* const swap_chain) {

    return 0;
}

RHI_VIEW* vk_swap_chain_create_view(const RHI_DEVICE* const device, const RHI_SWAP_CHAIN* const swap_chain, 
	const RHI_MEMORY_DESCRIPTOR_SLOT* memory_descriptor,
	resource_format format, const size_t buffer_id) {
	return  nullptr;
}
