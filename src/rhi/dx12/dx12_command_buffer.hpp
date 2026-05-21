#ifndef __dx12_command_buffer_hpp__
#define __dx12_command_buffer_hpp__

#include "dx12_rhi.hpp"

std::unique_ptr<RHI_COMMAND_BUFFER> dx12_command_buffer_create_for_copy(const RHI_COMMAND_BUFFER_DESC& desc);
std::unique_ptr<RHI_COMMAND_BUFFER> dx12_command_buffer_create_for_compute(const RHI_COMMAND_BUFFER_DESC& desc);
std::unique_ptr<RHI_COMMAND_BUFFER> dx12_command_buffer_create_for_render(const RHI_COMMAND_BUFFER_DESC& desc);
void dx12_command_buffer_record(RHI_COMMAND_BUFFER& command_buffer,
								fptr_command_buffer_on_record callback);

void dx12_command_buffer_draw_triangle_list(RHI_COMMAND_BUFFER& command_buffer, RHI_VERTEX_BUFFER& vb,
	RHI_INDEX_BUFFER* ib);
void dx12_command_buffer_ray_trace(RHI_DEVICE& device, RHI_COMMAND_BUFFER& command_buffer, RHI_TEXTURE_2D& render_target, RHI_RT_PIPELINE& pipeline, RHI_BUFFER& bvh_instances, RHI_BUFFER& sbt);


void dx12_command_buffer_reset_resource_state(RHI_COMMAND_BUFFER& command_buffer, RHI_RESOURCE& resource);

void dx12_command_buffer_copy_texture(RHI_COMMAND_BUFFER& command_buffer, RHI_TEXTURE_2D& dest_texture, RHI_TEXTURE_2D& src_texture);

#endif