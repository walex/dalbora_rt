//#include "vk_swap_chain.hpp"
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
