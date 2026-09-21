#ifndef __vk_rhi_hpp__
#define __vk_rhi_hpp__

#include "rhi.hpp"
#include "volk.h"

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
	}
};

struct VK_FENCE
	: public RHI_FENCE
	, public VK_NON_DISPATCHABLE_HANDLE<VkFence> {

	~VK_FENCE() {
		ASSERT_PTR(this->parent_device);
		vkDestroyFence(*this->parent_device, *this, nullptr);
	}
};

void vk_rhi_init();
void vk_rhi_end();
RHI_APP_INSTANCE vk_rhi_get_app_instance();

#endif