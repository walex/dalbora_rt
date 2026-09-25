#ifndef __rhi_command_allocator_pool_hpp__
#define __rhi_command_allocator_pool_hpp__

#include "rhi_types.hpp"

RHI_COMMAND_ALLOCATOR_POOL* rhi_command_allocator_pool_create(const RHI_DEVICE* const device, const size_t elements,
	queue_type type);
void rhi_command_allocator_pool_clean(RHI_COMMAND_ALLOCATOR_POOL* const pool);
RHI_COMMAND_ALLOCATOR* rhi_command_allocator_pool_acquire(RHI_COMMAND_ALLOCATOR_POOL* const pool);
void rhi_command_allocator_pool_release(RHI_COMMAND_ALLOCATOR_POOL* const pool, RHI_COMMAND_ALLOCATOR* const allocator);

#ifdef __cplusplus
#include "rhi_impl.hpp"
#include "rhi_command_allocator.hpp"
class RhiDevice;
class RhiCommandAllocatorPool 
	: public RhiImpl<RHI_COMMAND_ALLOCATOR_POOL>
	, ICreateRhiObject<const RhiDevice&, const size_t, const queue_type> {

public:
	IMPLEMENT_MOVABLE_CLASS(RhiCommandAllocatorPool);
	RhiCommandAllocator aquire();
	void release(RhiCommandAllocator& allocator);
	
	static void Initialize(const RhiDevice& device, const size_t elements, const queue_type type);
	static RhiCommandAllocatorPool& get(queue_type type);
private:
	RhiCommandAllocatorPool();
	void create(const RhiDevice& device, const size_t elements, const queue_type type);
	
	static std::unique_ptr<RhiCommandAllocatorPool> s_command_allocator_pool[queue_type_count];
};

#endif

#endif