#ifndef __vk_rhi_hpp__
#define __vk_rhi_hpp__

#include "rhi.hpp"

#define VK_KHR_swapchain
#ifdef WINDOWS_PLATFORM
	//#define VOLK_IMPLEMENTATION
	#define VK_USE_PLATFORM_WIN32_KHR
	//#define VK_KHR_win32_surface
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

void vk_rhi_init();
void vk_rhi_end();
RHI_APP_INSTANCE vk_rhi_get_app_instance();

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
		vkDestroyDevice(*this, nullptr);
	}
	VkPhysicalDevice physical_device;
	int32_t queue_family_index[queue_type_count] = { -1, -1, -1 };
};

template <typename T>
struct VK_NON_DISPATCHABLE_HANDLE
	: public VK_HANDLE<T> {
	virtual ~VK_NON_DISPATCHABLE_HANDLE() = default;
	const VK_DEVICE* parent_device = nullptr;
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
		ASSERT_PTR(this->memory_device);
		vkFreeMemory(*this->parent_device, this->memory_device, nullptr);
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
	: public RHI_MEMORY_DESCRIPTOR_SLOT {

	RHI_VOID_PTR memory_ptr = nullptr;
};

struct VK_MEMORY_POOL 
	: public VK_NON_DISPATCHABLE_HANDLE<VkDeviceMemory> {

	~VK_MEMORY_POOL() {
		ASSERT_PTR(this->parent_device);
		vkFreeMemory(*this->parent_device, *this, nullptr);
	}
};

struct VK_PIPELINE_STAGE_SYNC {
	virtual ~VK_PIPELINE_STAGE_SYNC() = default;
	VkSemaphore semaphore = VK_NULL_HANDLE;
	pipeline_stage stage = pipeline_stage_none;
};

#define MAX_SYNC_OBJECTS 16
struct VK_COMMAND_QUEUE
	: public RHI_COMMAND_QUEUE
	, public VK_NON_DISPATCHABLE_HANDLE<VkQueue> {

	~VK_COMMAND_QUEUE() {
		// No need to destroy VkQueue, as it is managed by the VkDevice
	}
	// can we use as generic for all graohics apis?
	size_t sync_objects_count = 0;
	VK_PIPELINE_STAGE_SYNC sync_objects[MAX_SYNC_OBJECTS];
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
		vkDeviceWaitIdle(*this->parent_device);
		vkDestroySwapchainKHR(*this->parent_device, *this, nullptr);
		ASSERT_PTR(this->native_surface);
		vkDestroySurfaceKHR(static_cast<VkInstance>(vk_rhi_get_app_instance()), this->native_surface, nullptr);
		if (this->buffers_count > 0) {
			for (size_t i = 0; i < this->buffers_count; ++i) {
				ASSERT_PTR(this->render_complete_sync[i]);
				ASSERT_PTR(this->next_image_sync[i]);
				vkDestroySemaphore(*this->parent_device, this->next_image_sync[i], nullptr);
				vkDestroySemaphore(*this->parent_device, this->render_complete_sync[i], nullptr);
			}
			delete[] this->next_image_sync;
			delete[] this->render_complete_sync;
		}
	}
	VkSurfaceKHR native_surface = VK_NULL_HANDLE;
	VkSemaphore* next_image_sync;
	VkSemaphore* render_complete_sync;
};

struct VK_FENCE
	: public RHI_FENCE
	, public VK_NON_DISPATCHABLE_HANDLE<VkSemaphore> {

	~VK_FENCE() {
		ASSERT_PTR(this->parent_device);
		vkDestroySemaphore(*this->parent_device, *this, nullptr);
	}
};


struct VK_IMAGE_VIEW
	: public RHI_VIEW
	, public VK_NON_DISPATCHABLE_HANDLE<VkImageView> {

	~VK_IMAGE_VIEW() {
		ASSERT_PTR(this->parent_device);
		vkDestroyImageView(*this->parent_device, *this, nullptr);
	}
};

struct VK_VIEW
	: public RHI_VIEW
	, public VK_NON_DISPATCHABLE_HANDLE<VkBufferView> {

	~VK_VIEW() {
		ASSERT_PTR(this->parent_device);
		vkDestroyImageView(*this->parent_device, *this, nullptr);
	}
};

struct VK_TEXTURE_2D
	: public VK_NON_DISPATCHABLE_HANDLE<VkImage>
	, public RHI_TEXTURE_2D
	, public RHI_BUFFER {

	~VK_TEXTURE_2D() {
		ASSERT_PTR(this->parent_device);
		vkDestroyImage(*this->parent_device, *this, nullptr);
	}
};

// FixME: should inherit from VK_TEXTURE_2D but blocks destructor
struct VK_SWAP_CHAIN_TEXTURE_2D
	: public VK_NON_DISPATCHABLE_HANDLE<VkImage>
	, public RHI_TEXTURE_2D
	, public RHI_BUFFER {

	~VK_SWAP_CHAIN_TEXTURE_2D() = default;
};

struct VK_RENDER_PASS
	: public RHI_RENDER_PASS {
};

struct VK_COMMAND_ALLOCATOR
	: public VK_NON_DISPATCHABLE_HANDLE<VkCommandPool>
	, public RHI_COMMAND_ALLOCATOR {


	~VK_COMMAND_ALLOCATOR() {

		ASSERT_PTR(this->parent_device);
		vkDestroyCommandPool(*this->parent_device, *this, nullptr);
	}
};

// VK_BVH -> VkAccelerationStructureKHR  ??

constexpr VkFormat vk_resource_format_type[resource_format_count] = {
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

constexpr VkPipelineStageFlagBits2 vk_pipeline_stages[pipeline_stage_count] = {
		VK_PIPELINE_STAGE_2_NONE,
		VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
		VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT,
		VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT,
		VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT,
		VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT,
		VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT,
		VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT,
		VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT,
		VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_EXT,
		VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_EXT,
		VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
		VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
		VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
		VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
		VK_PIPELINE_STAGE_2_COPY_BIT,
		VK_PIPELINE_STAGE_2_CLEAR_BIT,
		VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR,
		VK_PIPELINE_STAGE_2_PRE_RASTERIZATION_SHADERS_BIT
};



#endif