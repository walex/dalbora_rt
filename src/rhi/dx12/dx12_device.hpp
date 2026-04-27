#ifndef __dx12_device_hpp__
#define __dx12_device_hpp__

#include "dx12_rhi.hpp"

std::unique_ptr<RHI_OBJECT> dx12_create_device(const RHI_DEVICE_DESC& desc);

#endif