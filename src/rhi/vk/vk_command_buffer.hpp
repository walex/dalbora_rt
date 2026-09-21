#ifndef __vk_command_buffer_hpp__
#define __vk_command_buffer_hpp__

#include "vk_rhi.hpp"

RHI_COMMAND_BUFFER* vk_command_buffer_create_for_copy(
	const RHI_COMMAND_BUFFER_DESC* const desc);
RHI_COMMAND_BUFFER* vk_command_buffer_create_for_compute(
	const RHI_COMMAND_BUFFER_DESC* const desc);
RHI_COMMAND_BUFFER* vk_command_buffer_create_for_render(
	const RHI_COMMAND_BUFFER_DESC* const desc);
void vk_command_buffer_record(
	RHI_COMMAND_BUFFER* const command_buffer,
	fptr_command_buffer_on_record callback);
void vk_command_buffer_draw_triangle_list(
	RHI_COMMAND_BUFFER* const command_buffer,
	RHI_BUFFER* const vb,
	RHI_BUFFER* const ib);
void vk_command_buffer_ray_trace(
	RHI_COMMAND_BUFFER* const command_buffer,
	RHI_TEXTURE_2D* const render_target,
	const RHI_SBT_TABLE* const sbt);
void vk_command_buffer_copy_texture(
	RHI_COMMAND_BUFFER* const command_buffer,
	RHI_TEXTURE_2D* const dest_texture,
	const RHI_TEXTURE_2D* const src_texture);


#endif