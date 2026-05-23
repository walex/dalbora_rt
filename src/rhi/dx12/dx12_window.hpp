#ifndef __dx12_window_h__
#define __dx12_window_h__

#include "dx12_rhi.hpp"

RHI_WINDOW* dx12_window_create(const RHI_WINDOW_DESC* const desc);
void dx12_window_main_loop(const RHI_WINDOW* const window);

#endif
