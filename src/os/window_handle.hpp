#ifndef __window_handle_h__
#define __window_handle_h__

#include "rhi.hpp"

RHI_WINDOW* window_handle_create(const RHI_WINDOW_DESC* const desc);
void window_handle_main_loop(RHI_WINDOW* const window);

#endif