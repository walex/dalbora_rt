#include "vk_rhi.hpp"
#include "vk_device.hpp"
#include "vk_heap.hpp"
#include "vk_command_queue.hpp"
#include "vk_command_allocator.hpp"
#include "vk_command_buffer.hpp"
#include "vk_swap_chain.hpp"
#include "vk_render_pass.hpp"
#include "vk_pipeline_layout.hpp"
#include "vk_buffers.hpp"
#include "vk_raster_pipeline.hpp"

VkInstance create_vk_instance() {

    VkApplicationInfo app_info{};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_4;

    // instance extensions
    const char* extensions[] = {
        VK_KHR_SURFACE_EXTENSION_NAME,        // "VK_KHR_surface"
        VK_PLATFORM_KHR_SURFACE_EXTENSION_NAME   // "VK_KHR_win32_surface"
    };

    VkInstanceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.enabledExtensionCount = 2;
    create_info.ppEnabledExtensionNames = extensions;
    create_info.pApplicationInfo = &app_info;

#if defined(DEBUG)
    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };
    create_info.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    create_info.ppEnabledLayerNames = validationLayers.data();
#endif // DEBUG

    VkInstance instance;
    if (vkCreateInstance(&create_info, nullptr, &instance) != VK_SUCCESS) {
        throw std::runtime_error("¡Error al crear la instancia de Vulkan!");
    }

    volkLoadInstance(instance);

    return instance;
}

static VkInstance app_instance = nullptr;

void vk_rhi_init() {

    if (volkInitialize() != VK_SUCCESS) {
        throw std::runtime_error("Error: cannot initialize volk.");
    }

    app_instance = create_vk_instance();

    // device
    rhi_create_device = &vk_device_create;

    // heap
    rhi_memory_resource_create = &vk_memory_resource_create;
    rhi_memory_resource_get_descriptor = &vk_memory_resource_get_descriptor;

    // command allocator
    rhi_command_allocator_create = &vk_command_allocator_create;

    // command queue
    rhi_command_queue_create = &vk_command_queue_create;
    rhi_command_queue_execute = &vk_command_queue_execute;

    // swap chain api
    rhi_swap_chain_create = &vk_swap_chain_create;
    rhi_swap_chain_create_view = &vk_swap_chain_create_view;
    rhi_swap_chain_present = &vk_swap_chain_present;
    rhi_swap_chain_get_current_buffer_id = &vk_swap_chain_get_current_buffer_id;

    // command buffer
    rhi_command_buffer_create = &vk_command_buffer_create;
    rhi_command_buffer_record = &vk_command_buffer_record;
	rhi_command_buffer_draw_triangle_list = &vk_command_buffer_draw_triangle_list;

    // render pass
    rhi_render_pass_create = &vk_render_pass_create;
    rhi_render_pass_execute_raster_mode = &vk_render_pass_execute_raster_mode;

    // pipeline
    rhi_pipeline_layout_create = &vk_pipeline_layout_create;
	rhi_raster_pipeline_create = &vk_raster_pipeline_create;

    // buffers
    rhi_buffers_create_raw = &vk_buffers_create_linear;
    rhi_buffers_create_vertices = &vk_buffers_create_vertices;
    rhi_buffers_create_indices = &vk_buffers_create_indices;
    rhi_buffers_create_depth = &vk_buffers_create_depth;
    rhi_buffers_create_constant = &vk_buffers_create_constant;
    rhi_buffers_map_open = &vk_buffers_map_open;
    rhi_buffers_map_close = &vk_buffers_map_close;

    rhi_buffers_gpu_upload_region = &vk_buffers_gpu_upload_region;
    rhi_buffers_gpu_upload = &vk_buffers_gpu_upload;
    rhi_buffers_gpu_download_region = &vk_buffers_gpu_download_region;
    rhi_buffers_gpu_download = &vk_buffers_gpu_download;
    rhi_buffers_map_write = &vk_buffers_map_write;
    rhi_buffers_map_read = &vk_buffers_map_read;
    rhi_buffers_create_view = &vk_buffers_create_view;
    rhi_buffers_update_view = &vk_buffers_update_view;

   // rhi_texture_2d_gpu_upload = &vk_texture_2d_gpu_upload;

   // rhi_texture_2d_create = &vk_texture_2d_create;
   // rhi_sampler_create = &vk_sampler_create;

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