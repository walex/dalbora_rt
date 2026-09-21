#ifndef __vk_rhi_hpp__
#define __vk_rhi_hpp__

#include "rhi.hpp"
#include "volk.h"

struct VK_DEVICE;

template <typename T>
struct VK_HANDLE 
	: public RHI_HANDLE {

	VK_HANDLE(const VkDevice device = nullptr) : parent_device(device) {}
	virtual ~VK_HANDLE() = default;
	void set_handle(RHI_VOID_PTR h) override {

		this->handle = static_cast<T>(h);
	}

	RHI_VOID_PTR get_handle() const override {
		return static_cast<RHI_VOID_PTR>(handle);
	}
	T handle;
	VkDevice parent_device;
};

struct VK_DEVICE 
	: public RHI_DEVICE
	, public VK_HANDLE<VkDevice> {
	
	~VK_DEVICE() {
		vkDestroyDevice(*this, nullptr);
	}
	VkPhysicalDevice physical_device;
};

struct VK_MEMORY_DESCRIPTOR
	: public RHI_MEMORY_DESCRIPTOR
	, public VK_HANDLE<VkBuffer> {

	~VK_MEMORY_DESCRIPTOR() {
		this->release();
	}
	void release() {
		this->unmap();
		ASSERT_PTR(this->parent_device);
		vkDestroyBuffer(this->parent_device, *this, nullptr);
		this->memory_device = VK_NULL_HANDLE;
	}
	void map() {
		this->unmap();
		ASSERT_PTR(this->parent_device);
		vkMapMemory(this->parent_device, memory_device, 0, VK_WHOLE_SIZE, 0, &mapped_memory);
	}
	void unmap() {
		if (mapped_memory == nullptr)
			return;
		ASSERT_PTR(this->parent_device);
		vkUnmapMemory(this->parent_device, memory_device);
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
	: public VK_HANDLE<VkDeviceMemory> {

	~VK_MEMORY_POOL() {
		ASSERT_PTR(this->parent_device);
		vkFreeMemory(this->parent_device, *this, nullptr);
	}
};

void vk_rhi_init();
void vk_rhi_end();
RHI_APP_INSTANCE vk_rhi_get_app_instance();

#endif