#ifndef __dx12_fence_hpp__
#define __dx12_fence_hpp__

#include "dx12_hal.hpp"

std::unique_ptr<HAL_OBJECT> dx12_create_fence(const HAL_FENCE_DESC& desc);

#endif