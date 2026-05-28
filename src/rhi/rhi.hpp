#ifndef __rhi_hpp__
#define __rhi_hpp__

#include <stdint.h>
#include <Eigen/Dense>

#include "rhi_types.hpp"
#include "rhi_creation_descriptors.hpp"
#include "file_system.hpp"
#include "strings.hpp"

void rhi_init(device_type dt);
void rhi_end();

// window api
inline RHI_WINDOW* (*rhi_create_window)(const RHI_WINDOW_DESC* const desc);
inline void (*rhi_window_main_loop)(RHI_WINDOW* const handle);

inline RHI_DEVICE* (*rhi_create_device)(const RHI_DEVICE_DESC* const desc);

// swap chain api
inline RHI_SWAP_CHAIN* (*rhi_swap_chain_create)(const RHI_SWAP_CHAIN_DESC* const swpc_desc);
inline void (*rhi_swap_chain_present)(const RHI_SWAP_CHAIN* const);
inline const RHI_VIEW* const (*rhi_swap_chain_get_surface)(const RHI_SWAP_CHAIN* const swap_chain, const size_t surface_index);
inline uint32_t (*rhi_swap_chain_get_current_buffer_id)(const RHI_SWAP_CHAIN* const swap_chain);
inline void (*rhi_swap_chain_copy_direct)(RHI_SWAP_CHAIN& swap_chain, RHI_TEXTURE_2D& render_target);

// command queue
inline RHI_COMMAND_QUEUE* (*rhi_command_queue_create_for_render)(const RHI_COMMAND_QUEUE_DESC* const queue_desc);
inline RHI_COMMAND_QUEUE* (*rhi_command_queue_create_for_compute)(const RHI_COMMAND_QUEUE_DESC* const queue_desc);
inline RHI_COMMAND_QUEUE* (*rhi_command_queue_create_for_copy)(const RHI_COMMAND_QUEUE_DESC* const queue_desc);
inline void (*rhi_command_queue_execute)(RHI_COMMAND_QUEUE* const command_queue, bool wait_completion, fptr_command_queue_on_execute callback);

// command buffer
inline RHI_COMMAND_BUFFER* (*rhi_command_buffer_create_for_copy)(const RHI_COMMAND_BUFFER_DESC* const cb_desc);
inline RHI_COMMAND_BUFFER* (*rhi_command_buffer_create_for_compute)(const RHI_COMMAND_BUFFER_DESC* const cb_desc);
inline RHI_COMMAND_BUFFER* (*rhi_command_buffer_create_for_render)(const RHI_COMMAND_BUFFER_DESC* const cb_desc);
inline void (*rhi_command_buffer_record)(RHI_COMMAND_BUFFER* const command_buffer,
										 fptr_command_buffer_on_record callback);
inline void (*rhi_command_buffer_draw_triangle_list)(RHI_COMMAND_BUFFER* const command_buffer, const RHI_BUFFER* const vb,
	const RHI_BUFFER* const ib);
inline void (*rhi_command_buffer_copy_texture)(RHI_COMMAND_BUFFER* const command_buffer, RHI_TEXTURE_2D* const dest_texture,
	const RHI_TEXTURE_2D* const src_texture);

// buffers
inline RHI_BUFFER* (*rhi_buffers_create_raw)(const RHI_BUFFER_DESC* const desc);
inline RHI_BUFFER* (*rhi_buffers_create_vertices)(const RHI_VERTEX_BUFFER_DESC* const desc);
inline RHI_BUFFER* (*rhi_buffers_create_indices)(const RHI_INDEX_BUFFER_DESC* const desc);
inline RHI_BUFFER* (*rhi_buffers_create_depth)(const RHI_BUFFER_2D_DESC* const desc);
inline RHI_BUFFER* (*rhi_buffers_create_constant)(const RHI_BUFFER_DESC* const desc);
inline RHI_VOID_PTR (*rhi_buffers_map_open)(RHI_BUFFER* const shared_buffer,
	const size_t offset,
	const size_t length);
inline void (*rhi_buffers_map_close)(RHI_BUFFER* const shared_buffer,
	const size_t offset,
	const size_t length);
inline void (*rhi_buffers_gpu_upload_region)(RHI_COMMAND_BUFFER* const command_buffer, const RHI_BUFFER* const src_buffer,
	RHI_BUFFER* const dest_buffer, const size_t offset_src,
	const size_t offset_dest, const size_t length);
inline void (*rhi_buffers_gpu_upload)(RHI_COMMAND_BUFFER* const command_buffer, const RHI_BUFFER* const src_buffer,
	RHI_BUFFER* const dest_buffer);
inline void (*rhi_buffers_gpu_download_region)(RHI_COMMAND_BUFFER* const command_buffer, const RHI_BUFFER* const src_buffer,
	RHI_BUFFER* const dest_buffer, const size_t offset_src,
	const size_t offset_dest, const size_t length);
inline void (*rhi_buffers_gpu_download)(RHI_COMMAND_BUFFER* const command_buffer, const RHI_BUFFER* const src_buffer,
	RHI_BUFFER* const dest_buffer);
inline void (*rhi_buffers_map_write)(RHI_BUFFER* const shared_buffer, RHI_VOID_PTR data,
									 size_t offset, size_t length);
inline void (*rhi_buffers_map_read)(RHI_BUFFER* const shared_buffer, RHI_VOID_PTR* const data,
									size_t offset, size_t length);
inline RHI_VIEW* (*rhi_buffers_create_view)(const RHI_VIEW_DESC* const desc);
inline void (*rhi_buffers_update_view)(const RHI_DEVICE* const device, RHI_VIEW* const view, const RHI_BUFFER* const buffer);
inline void (*rhi_texture_2d_gpu_upload)(RHI_COMMAND_BUFFER* const command_buffer,
	const RHI_BUFFER* const src_buffer,
	RHI_TEXTURE_2D* const dest_buffer);
inline RHI_TEXTURE_2D* (*rhi_texture_2d_create)(const RHI_TEXTURE_2D_DESC* const tex_desc);
inline RHI_SAMPLER*(*rhi_sampler_create)(const RHI_RT_SAMPLER_DESC* const des);

// pipeline
inline RHI_RT_PIPELINE* (*rhi_rt_pipeline_create)(const RHI_RT_PIPELINE_DESC* const desc);
inline RHI_RASTER_PIPELINE* (*rhi_raster_pipeline_create)(const RHI_RASTER_PIPELINE_DESC* const pipeline_desc);
inline RHI_SBT_TABLE* (*rhi_rt_pipeline_create_sbt)(const RHI_DEVICE* const device, const RHI_RT_SBT_DESC* const desc, const RHI_RT_PIPELINE* const pipeline);

// pipeline layout
inline RHI_PIPELINE_LAYOUT* (*rhi_pipeline_layout_create)(const RHI_PIPELINE_LAYOUT_DESC* const desc);
inline RHI_COMPILED_SHADER_BUFFER* (*rhi_shaders_compiler_compile)(const char* const file,
	const char* const entry,
	const char* const target);
inline void (*rhi_shaders_compiler_set_folder)(const char *const folder);

// render pass api
inline RHI_RENDER_PASS* (*rhi_render_pass_create)(const RHI_RENDER_PASS_DESC* const desc);
inline void (*rhi_render_pass_execute_raster_mode)(const RHI_RENDER_PASS* const render_pass,
	RHI_COMMAND_BUFFER* const command_buffer,
	fptr_render_pass_on_execute callback);
inline void (*rhi_render_pass_execute_rt_mode)(const RHI_RENDER_PASS* const render_pass, RHI_COMMAND_BUFFER* const command_buffer,
	fptr_render_pass_on_execute callback);

// rt
inline RHI_RT_BVH* (*rhi_rt_bvh_create)(const RHI_RT_BVH_DESC* const desc);
inline RHI_BUFFER*(*rhi_rt_bvh_build_geometry_instances)(const RT_GEOMETRY_INSTANCES_DESC* const desc);
inline void (*rhi_command_buffer_ray_trace)(RHI_COMMAND_BUFFER* const command_buffer,
	RHI_TEXTURE_2D* const render_target, const RHI_BUFFER* const bvh_instances, 
	const RHI_SBT_TABLE* const sbt);

#endif