#ifndef __dx12_fence_hpp__
#define __dx12_fence_hpp__

#include "dx12_rhi.hpp"

std::unique_ptr<RHI_FENCE> dx12_fence_create(const RHI_FENCE_DESC& desc);

#endif