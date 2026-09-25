#include "rhi_command_allocator_pool.hpp"
#include "rhi_device.hpp"

RHI_COMMAND_ALLOCATOR_POOL* rhi_command_allocator_pool_create(const RHI_DEVICE* const device, const size_t elements,
	queue_type type) {
	
	ASSERT_PTR(device);
	RHI_COMMAND_ALLOCATOR_POOL* pool = new RHI_COMMAND_ALLOCATOR_POOL();
	ASSERT_PTR(pool);
	for (size_t i = 0; i < elements; ++i) {
		RHI_COMMAND_ALLOCATOR* allocator = rhi_command_allocator_create(device, type);
		ASSERT_PTR(allocator);
		pool->allocators.push(allocator);
	}
	return pool;
}

void rhi_command_allocator_pool_clean(RHI_COMMAND_ALLOCATOR_POOL* const pool) {

	ASSERT_PTR(pool);
	pool->allocators.pop_all([](RHI_COMMAND_ALLOCATOR*& allocator) {
		delete allocator;
		});
}

RHI_COMMAND_ALLOCATOR* rhi_command_allocator_pool_acquire(RHI_COMMAND_ALLOCATOR_POOL* const pool) {
	ASSERT_PTR(pool);
	RHI_COMMAND_ALLOCATOR* allocator = nullptr;
	if (pool->allocators.pop(allocator, 0)) {
		return allocator;
	}
	throw std::runtime_error("No available command allocator in the pool.");
}

void rhi_command_allocator_pool_release(RHI_COMMAND_ALLOCATOR_POOL* const pool, RHI_COMMAND_ALLOCATOR* const allocator) {
	ASSERT_PTR(pool);
	ASSERT_PTR(allocator);
	pool->allocators.push(allocator);
}

#ifdef __cplusplus

std::unique_ptr<RhiCommandAllocatorPool> RhiCommandAllocatorPool::s_command_allocator_pool[queue_type_count];

void RhiCommandAllocatorPool::Initialize(const RhiDevice& device, const size_t elements, const queue_type type) {

	s_command_allocator_pool[type].reset(new RhiCommandAllocatorPool());
	s_command_allocator_pool[type]->create(device, elements, type);
}

RhiCommandAllocatorPool& RhiCommandAllocatorPool::get(queue_type type) {
	if (!s_command_allocator_pool[type])
		throw std::runtime_error("Command allocator pool is not initialized.");
	return *s_command_allocator_pool[type];
}

RhiCommandAllocatorPool::RhiCommandAllocatorPool()
	: RhiImpl<RHI_COMMAND_ALLOCATOR_POOL>(nullptr) {
}

void RhiCommandAllocatorPool::create(const RhiDevice& device, const size_t elements, const queue_type type) {
	this->set_handle(rhi_command_allocator_pool_create(device, elements, type));
}
RhiCommandAllocator RhiCommandAllocatorPool::aquire() {
	return RhiCommandAllocator(rhi_command_allocator_pool_acquire(*this), *this);
}
void RhiCommandAllocatorPool::release(RhiCommandAllocator& allocator) {
	rhi_command_allocator_pool_release(*this, allocator);
}

#endif