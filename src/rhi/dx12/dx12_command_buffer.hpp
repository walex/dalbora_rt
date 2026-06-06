#ifndef __dx12_command_buffer_hpp__
#define __dx12_command_buffer_hpp__

#include "dx12_rhi.hpp"

RHI_COMMAND_BUFFER* dx12_command_buffer_create_for_copy(
	const RHI_COMMAND_BUFFER_DESC* const desc);
RHI_COMMAND_BUFFER* dx12_command_buffer_create_for_compute(
	const RHI_COMMAND_BUFFER_DESC* const desc);
RHI_COMMAND_BUFFER* dx12_command_buffer_create_for_render(
	const RHI_COMMAND_BUFFER_DESC* const desc);
void dx12_command_buffer_record(
	RHI_COMMAND_BUFFER* const command_buffer,
	fptr_command_buffer_on_record callback);
void dx12_command_buffer_draw_triangle_list(
	RHI_COMMAND_BUFFER* const command_buffer, 
	const RHI_BUFFER* const vb,
	const RHI_BUFFER* const ib);
void dx12_command_buffer_ray_trace(
	RHI_COMMAND_BUFFER* const command_buffer,
	RHI_TEXTURE_2D* const render_target, 
	const RHI_SBT_TABLE* const sbt);
void dx12_command_buffer_copy_texture(
	RHI_COMMAND_BUFFER* const command_buffer, 
	RHI_TEXTURE_2D* const dest_texture, 
	const RHI_TEXTURE_2D* const src_texture);
void dx12_command_buffer_resource_transition(
	ID3D12GraphicsCommandList* const i_command_buffer,
	const DX_RESOURCE* const resource_impl[],
	const D3D12_RESOURCE_STATES states[],
	const bool restore[],
	const size_t count,
	std::function<void()> cb);
#endif