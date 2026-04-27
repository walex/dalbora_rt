#ifndef __dx12_device_hpp__
#define __dx12_device_hpp__

#include "dx12_hal.hpp"

std::unique_ptr<HAL_OBJECT> dx12_create_device(const HAL_DEVICE_DESC& desc);

#endif