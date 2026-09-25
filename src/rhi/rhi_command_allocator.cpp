#include "rhi_command_allocator.hpp"
#include "rhi_command_allocator_pool.hpp"

RhiCommandAllocator::RhiCommandAllocator(RHI_COMMAND_ALLOCATOR* handle, RhiCommandAllocatorPool& pool)
	: RhiImpl<RHI_COMMAND_ALLOCATOR>(handle)
	, m_pool(pool) {
	ASSERT_PTR(handle);
}

RhiCommandAllocator::~RhiCommandAllocator() {
	if (this->empty() == false)
		m_pool.release(*this);
}