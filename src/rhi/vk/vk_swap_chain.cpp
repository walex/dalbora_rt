#include "vk_swap_chain.hpp"
#include "vk_heap.hpp"

//TODO: move to helpers
size_t vk_bytes_per_pixel_from_format(VkFormat format)
{
    switch (format)
    {
    case VK_FORMAT_R8_UNORM:
        return 1;

    case VK_FORMAT_R8G8_UNORM:
        return 2;

    case VK_FORMAT_R8G8B8_UNORM:
    case VK_FORMAT_R8G8B8_SRGB:
        return 3;

    case VK_FORMAT_R8G8B8A8_UNORM:
    case VK_FORMAT_R8G8B8A8_SRGB:
    case VK_FORMAT_B8G8R8A8_UNORM:
    case VK_FORMAT_B8G8R8A8_SRGB:
        return 4;

    case VK_FORMAT_R16_UNORM:
    case VK_FORMAT_R16_SFLOAT:
        return 2;

    case VK_FORMAT_R16G16_UNORM:
    case VK_FORMAT_R16G16_SFLOAT:
        return 4;

    case VK_FORMAT_R16G16B16A16_UNORM:
    case VK_FORMAT_R16G16B16A16_SFLOAT:
        return 8;

    case VK_FORMAT_R32_SFLOAT:
        return 4;

    case VK_FORMAT_R32G32_SFLOAT:
        return 8;

    case VK_FORMAT_R32G32B32_SFLOAT:
        return 12;

    case VK_FORMAT_R32G32B32A32_SFLOAT:
        return 16;

    case VK_FORMAT_D16_UNORM:
        return 2;

    case VK_FORMAT_D32_SFLOAT:
        return 4;

    case VK_FORMAT_D24_UNORM_S8_UINT:
        return 4;

    case VK_FORMAT_D32_SFLOAT_S8_UINT:
        return 8;

    default:
        return 0;
    }
}

typedef VkSurfaceKHR(*create_surface_func_ptr)(const RHI_WINDOW* const window, const RHI_VOID_PTR* const param);

#ifdef WINDOWS_PLATFORM
VkSurfaceKHR create_surface_win32(const RHI_WINDOW* const window, const RHI_VOID_PTR* const param) {

    VkInstance instance = static_cast<VkInstance>(rhi_get_app_instance());

    // Ensure window pointer and handle are valid
    if (!window || !window->handle) {
        printf("Invalid window or window->handle is NULL");
        return VK_NULL_HANDLE;
    }
    HWND hwnd = static_cast<HWND>(window->handle);
    if (hwnd == reinterpret_cast<HWND>(-1) || hwnd == nullptr) {
        printf("window->handle is invalid (NULL or -1)");
        return VK_NULL_HANDLE;
    }
    if (!IsWindow(hwnd)) {
        printf("HWND is not a valid window (IsWindow returned FALSE)");
        return VK_NULL_HANDLE;
    }
    HMODULE hinst = GetModuleHandle(NULL);
    if (!hinst) {
        printf("GetModuleHandle(NULL) returned NULL");
        return VK_NULL_HANDLE;
    }

    VkWin32SurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.flags = 0;
    createInfo.hinstance = hinst;
    createInfo.hwnd = hwnd;    

	// FIXMe: use vkCreateWin32SurfaceKHR directly
    // for some reason Volker returns an incorrect pointer
    static PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR = reinterpret_cast<PFN_vkCreateWin32SurfaceKHR>(
        vkGetInstanceProcAddr(instance, "vkCreateWin32SurfaceKHR")
        );

    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkResult result = vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &surface);

    if (result != VK_SUCCESS) {
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

    uint32_t width, height;

    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR
        (static_cast<VK_DEVICE*>(desc->device)->physical_device, surface, &surfaceCapabilities); 

	width = surfaceCapabilities.currentExtent.width;
	height = surfaceCapabilities.currentExtent.height;

	VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;

    createInfo.minImageCount = desc->buffer_count;
    createInfo.imageFormat = (desc->color_format != resource_format_none)
        ? vk_resource_format_type[desc->color_format]
        : VK_FORMAT_B8G8R8A8_SRGB;;
    createInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    createInfo.imageExtent = VkExtent2D{ width, height};
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_2_STORAGE_BIT_KHR;

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
	result->buffer_width = width;
	result->buffer_height = height;
    result->buffer_mip_count = 1;
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

    ASSERT_PTR(swap_chain);
    VK_SWAP_CHAIN* swap_chain_impl = static_cast<VK_SWAP_CHAIN*>(const_cast<RHI_SWAP_CHAIN*>(swap_chain));

    ASSERT_PTR(swap_chain_impl->parent_device);
    const VK_DEVICE* device_impl = swap_chain_impl->parent_device;

    uint32_t image_count = 0;
    vkGetSwapchainImagesKHR(*device_impl, *swap_chain_impl, &image_count, nullptr);

	ASSERT_EXPR(image_count > 0 && buffer_id < image_count);

    std::vector<VkImage> images(image_count);
    vkGetSwapchainImagesKHR(*device_impl, *swap_chain_impl, &image_count, images.data());
	ASSERT_EXPR(buffer_id < images.size());
    
	VkImage image = images[buffer_id];
    VkImageViewCreateInfo view_info{};
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.image = image;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = format != resource_format_none
        ? vk_resource_format_type[format]
        : VK_FORMAT_B8G8R8A8_SRGB;
    view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;

    VkImageView image_view;
    if (vkCreateImageView(*device_impl, &view_info, nullptr, &image_view) != VK_SUCCESS) {
        throw std::runtime_error("Error creating ImageView for Swapchain");
    }
    ASSERT_PTR(image_view);
    
    vk_memory_resource_write_image_descriptor(device_impl, memory_descriptor, &view_info);

    size_t offset = 0;
    size_t mip_count = swap_chain->buffer_mip_count;
    std::vector<RHI_TEXTURE_MIPS> mips(mip_count);
	size_t bytes_per_pixel = vk_bytes_per_pixel_from_format(vk_resource_format_type[format]);
    for (uint32_t mip = 0; mip < mip_count; ++mip)
    {
        RHI_TEXTURE_MIPS& tm = mips[mip];
        tm.offset = offset;
        tm.width = std::max(static_cast<size_t>(1), swap_chain->buffer_width >> mip);
        tm.height = std::max(static_cast<size_t>(1), swap_chain->buffer_height >> mip);
        tm.depth = 1;
        tm.num_rows = tm.height;
        tm.pitch = tm.width * bytes_per_pixel;
        tm.format = format;
        offset += tm.pitch * tm.num_rows;
    }

    size_t totalUploadSize = offset;

    VK_TEXTURE_2D* texture = new VK_TEXTURE_2D();
    ASSERT_PTR(texture);
    texture->set_handle(image);
    texture->hw_format = format;
    texture->width = swap_chain->buffer_width;
    texture->height = swap_chain->buffer_height;
    texture->hw_length = totalUploadSize;
    texture->mip_maps = std::move(mips);

    RHI_VIEW_DESC view_desc;
    view_desc.type = shader_view_type_render_target;
    view_desc.device = const_cast<RHI_DEVICE*>(device);
    view_desc.format = format;
    view_desc.memory_descriptor = memory_descriptor;
    view_desc.buffer = texture;
    VK_IMAGE_VIEW* result = new VK_IMAGE_VIEW();
    result->set_handle(image_view);
    result->buffer = make_releseable_observer_ptr<RHI_BUFFER>(texture);
    result->memory_descriptor = const_cast<RHI_MEMORY_DESCRIPTOR_SLOT*>(memory_descriptor);
    return result;
}
