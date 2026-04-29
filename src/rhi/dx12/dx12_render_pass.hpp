#ifndef __dx12_render_pass_hpp__
#define __dx12_render_pass_hpp__

#include "dx12_rhi.hpp"

std::unique_ptr<RHI_OBJECT> render_pass_create(RHI_OBJECT& image_buffer);

#endif