#ifndef __rhi_command_allocator_hpp__
#define __rhi_command_allocator_hpp__	

#include "rhi_impl.hpp"

class RhiCommandAllocatorPool;
class RhiCommandAllocator
	: public RhiImpl<RHI_COMMAND_ALLOCATOR> {
	friend class RhiCommandAllocatorPool;
public:
	IMPLEMENT_MOVABLE_CLASS(RhiCommandAllocator);
	~RhiCommandAllocator();
private:
	RhiCommandAllocator(RHI_COMMAND_ALLOCATOR* handle, RhiCommandAllocatorPool& pool);
	RhiCommandAllocatorPool& m_pool;
};

#endif