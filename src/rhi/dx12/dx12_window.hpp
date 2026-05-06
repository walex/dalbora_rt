#ifndef __dx12_window_h__
#define __dx12_window_h__

#include "dx12_rhi.hpp"

std::unique_ptr<RHI_WINDOW> dx12_window_create(const RHI_WINDOW_DESC& desc);
void dx12_window_main_loop(RHI_WINDOW& window);

#endif
