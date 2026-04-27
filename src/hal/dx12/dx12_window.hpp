#ifndef __dx12_window_h__
#define __dx12_window_h__

#include "dx12_hal.hpp"

std::unique_ptr<HAL_HANDLE> dx12_create_window(const HAL_WINDOW_DESC& desc);

#endif
