#ifndef __vk_render_pass_hpp__
#define __vk_render_pass_hpp__

#include "vk_rhi.hpp"

RHI_RENDER_PASS* vk_render_pass_create(const RHI_RENDER_PASS_DESC* const desc);
void vk_render_pass_execute_raster_mode(const RHI_RENDER_PASS* const render_pass, RHI_COMMAND_BUFFER* const command_buffer,
	fptr_render_pass_on_execute callback);

#endif