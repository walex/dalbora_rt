#ifndef __dx12_descriptor_pool_hpp__
#define __dx12_descriptor_pool_hpp__

#include "dx12_rhi.hpp"

std::unique_ptr<RHI_DESCRIPTOR_POOL> dx12_descriptor_pool_create(const RHI_DESCRIPTOR_POOL_DESC& desc);


#endif