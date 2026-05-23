#ifndef __dx12_render_pass_hpp__
#define __dx12_render_pass_hpp__

#include "dx12_rhi.hpp"

RHI_RENDER_PASS* dx12_render_pass_create(const RHI_RENDER_PASS_DESC* const desc);
void dx12_render_pass_execute_raster_mode(const RHI_RENDER_PASS* const render_pass, RHI_COMMAND_BUFFER* const command_buffer,
	fptr_render_pass_on_execute callback); 
void dx12_render_pass_execute_rt_mode(const RHI_RENDER_PASS* render_pass, RHI_COMMAND_BUFFER* const command_buffer,
	fptr_render_pass_on_execute callback);
#endif