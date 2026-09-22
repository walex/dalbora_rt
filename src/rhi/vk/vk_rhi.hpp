#ifndef __vk_rhi_hpp__
#define __vk_rhi_hpp__

#include "rhi.hpp"

#ifdef WINDOWS_PLATFORM
	#define VK_USE_PLATFORM_WIN32_KHR
	#include "volk.h"
	#define VK_PLATFORM_KHR_SURFACE_EXTENSION_NAME VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#endif
	
#ifdef LINUX_PLATFORM
	#define VK_USE_PLATFORM_WAYLAND_KHR
	#include "volk.h"
	#define VK_PLATFORM_KHR_SURFACE_EXTENSION_NAME VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME
	#include <wayland-client.h>
	#error "Vulkan support not yet implemented for linux"
#endif

#ifdef ANDROID_PLATFORM
	#define VK_USE_PLATFORM_ANDROID_KHR
	#include "volk.h"
	#define VK_PLATFORM_KHR_SURFACE_EXTENSION_NAME VK_KHR_ANDROID_SURFACE_EXTENSION_NAME
	#error "Vulkan support not yet implemented for android"
#endif

#ifndef VK_PLATFORM_KHR_SURFACE_EXTENSION_NAME
	#error "Vulkan support not yet implemented for this platform"
#endif


struct VK_DEVICE;

template <typename T>
struct VK_HANDLE 
	: public RHI_HANDLE {

	virtual ~VK_HANDLE() = default;
	void set_handle(RHI_VOID_PTR h) override {

		this->handle = static_cast<T>(h);
	}

	RHI_VOID_PTR get_handle() const override {
		return static_cast<RHI_VOID_PTR>(handle);
	}
	T handle;
};

struct VK_DEVICE 
	: public RHI_DEVICE
	, public VK_HANDLE<VkDevice> {
	
	~VK_DEVICE() {
		if (graphics_queue_command_pool != VK_NULL_HANDLE)
			vkDestroyCommandPool(*this, graphics_queue_command_pool, nullptr);
		if (compute_queue_command_pool != VK_NULL_HANDLE)
			vkDestroyCommandPool(*this, compute_queue_command_pool, nullptr);
		if (copy_queue_command_pool != VK_NULL_HANDLE)
			vkDestroyCommandPool(*this, copy_queue_command_pool, nullptr);
		vkDestroyDevice(*this, nullptr);
	}
	VkPhysicalDevice physical_device;
	VkCommandPool graphics_queue_command_pool = VK_NULL_HANDLE;
	VkCommandPool compute_queue_command_pool = VK_NULL_HANDLE;
	VkCommandPool copy_queue_command_pool = VK_NULL_HANDLE;
	uint32_t graphics_queue_family_index = -1;
	uint32_t compute_queue_family_index = -1;
	uint32_t copy_queue_family_index = -1;
};

template <typename T>
struct VK_NON_DISPATCHABLE_HANDLE
	: public VK_HANDLE<T> {
	virtual ~VK_NON_DISPATCHABLE_HANDLE() = default;
	VK_DEVICE* parent_device = nullptr;
};

struct VK_MEMORY_DESCRIPTOR
	: public RHI_MEMORY_DESCRIPTOR
	, public VK_NON_DISPATCHABLE_HANDLE<VkBuffer> {

	~VK_MEMORY_DESCRIPTOR() {
		this->release();
	}
	void release() {
		this->unmap();
		ASSERT_PTR(this->parent_device);
		vkDestroyBuffer(*this->parent_device, *this, nullptr);
		this->memory_device = VK_NULL_HANDLE;
	}
	void map() {
		this->unmap();
		ASSERT_PTR(this->parent_device);
		vkMapMemory(*this->parent_device, memory_device, 0, VK_WHOLE_SIZE, 0, &mapped_memory);
	}
	void unmap() {
		if (mapped_memory == nullptr)
			return;
		ASSERT_PTR(this->parent_device);
		vkUnmapMemory(*this->parent_device, memory_device);
		this->mapped_memory = nullptr;
	}
	VkDeviceMemory memory_device;
	RHI_VOID_PTR mapped_memory = nullptr;
};

struct VK_MEMORY_DESCRIPTOR_SLOT
	: public RHI_MEMORY_DESCRIPTOR_SLOT
	, public RHI_MEMORY_DESCRIPTOR {

	RHI_VOID_PTR memory_ptr = nullptr;
};

struct VK_MEMORY_POOL 
	: public VK_NON_DISPATCHABLE_HANDLE<VkDeviceMemory> {

	~VK_MEMORY_POOL() {
		ASSERT_PTR(this->parent_device);
		vkFreeMemory(*this->parent_device, *this, nullptr);
	}
};

struct VK_COMMAND_QUEUE
	: public RHI_COMMAND_QUEUE
	, public VK_NON_DISPATCHABLE_HANDLE<VkQueue> {

	VkSemaphore timeline_semaphore = VK_NULL_HANDLE;
	~VK_COMMAND_QUEUE() {
		// No need to destroy VkQueue, as it is managed by the VkDevice
	}
};

struct VK_COMMAND_BUFFER
	: public RHI_COMMAND_BUFFER
	, public VK_NON_DISPATCHABLE_HANDLE<VkCommandBuffer> {

	~VK_COMMAND_BUFFER() {
		ASSERT_PTR(this->parent_device);
		ASSERT_PTR(this->command_pool);
		vkFreeCommandBuffers(*this->parent_device, this->command_pool, 1, &handle);
	}
	VkCommandPool command_pool;
};

struct VK_SWAP_CHAIN
	: public RHI_SWAP_CHAIN
	, public VK_NON_DISPATCHABLE_HANDLE<VkSwapchainKHR> {
	~VK_SWAP_CHAIN() {
		ASSERT_PTR(this->parent_device);
		vkDestroySwapchainKHR(*this->parent_device, *this, nullptr);
		ASSERT_PTR(this->native_surface);
		vkDestroySurfaceKHR(*this->parent_device, this->native_surface, nullptr);
	}
	VkSurfaceKHR native_surface;
};

struct VK_FENCE
	: public RHI_FENCE
	, public VK_NON_DISPATCHABLE_HANDLE<VkFence> {

	~VK_FENCE() {
		ASSERT_PTR(this->parent_device);
		vkDestroyFence(*this->parent_device, *this, nullptr);
	}
};

constexpr VkFormat vk_resource_format_type[] = {
	VK_FORMAT_UNDEFINED,                // resource_format_none
	VK_FORMAT_R8_UINT,                  // resource_format_uint18
	VK_FORMAT_R16_UINT,                 // resource_format_uint16
	VK_FORMAT_R32_UINT,                 // resource_format_uint32
	VK_FORMAT_R8G8B8A8_UNORM,           // resource_format_R8G8B8A8
	VK_FORMAT_R32_SFLOAT,               // resource_format_float
	VK_FORMAT_R32G32_SFLOAT,            // resource_format_float2
	VK_FORMAT_R32G32B32_SFLOAT,         // resource_format_float3
	VK_FORMAT_R32G32B32A32_SFLOAT,      // resource_format_float4
	VK_FORMAT_D32_SFLOAT_S8_UINT,       // resource_format_d32_float_s8_uint
	VK_FORMAT_D24_UNORM_S8_UINT,        // resource_format_d24_norm_s8_uint
	VK_FORMAT_D32_SFLOAT,               // resource_format_32_float
	VK_FORMAT_D16_UNORM,                // resource_format_d16_norm
	VK_FORMAT_BC1_RGBA_UNORM_BLOCK      // resource_format_bc1_norm
};

void vk_rhi_init();
void vk_rhi_end();
RHI_APP_INSTANCE vk_rhi_get_app_instance();

#endif