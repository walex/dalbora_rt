#ifndef __dx12_render_pass_hpp__
#define __dx12_render_pass_hpp__

#include "dx12_rhi.hpp"

std::unique_ptr<RHI_OBJECT> dx12_render_pass_create(RHI_RENDER_PASS_DESC& desc);
void dx12_render_pass_begin(RHI_OBJECT& render_pass, RHI_OBJECT& command_buffer);
void dx12_render_pass_end(RHI_OBJECT& render_pass, RHI_OBJECT& command_queue, RHI_OBJECT& command_buffer);

#endif