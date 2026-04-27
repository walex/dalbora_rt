#ifndef __dx12_window_h__
#define __dx12_window_h__

#include "dx12_rhi.hpp"

std::unique_ptr<RHI_HANDLE> dx12_create_window(const RHI_WINDOW_DESC& desc);

#endif
