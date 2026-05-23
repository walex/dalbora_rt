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
inline void (*rhi_window_main_loop)(const RHI_WINDOW* const handle);

inline RHI_DEVICE* (*rhi_create_device)(const RHI_DEVICE_DESC* const desc);

// swap chain api
inline RHI_SWAP_CHAIN* (*rhi_swap_chain_create)(const RHI_SWAP_CHAIN_DESC* const swpc_desc);
inline void (*rhi_swap_chain_present)(const RHI_SWAP_CHAIN_DESC* const);
inline RHI_TEXTURE_2D* (*rhi_swap_chain_get_surface)(RHI_SWAP_CHAIN &swap_chain, int surface_index);
inline unsigned int (*rhi_swap_chain_get_current_buffer_id)(RHI_SWAP_CHAIN &swap_chain);
inline void (*rhi_swap_chain_copy_direct)(RHI_SWAP_CHAIN& swap_chain, RHI_TEXTURE_2D& render_target);

// command queue
inline RHI_COMMAND_QUEUE* (*rhi_command_queue_create_for_render)(const RHI_COMMAND_QUEUE_DESC &queue_desc);
inline RHI_COMMAND_QUEUE* (*rhi_command_queue_create_for_compute)(const RHI_COMMAND_QUEUE_DESC &queue_desc);
inline RHI_COMMAND_QUEUE* (*rhi_command_queue_create_for_copy)(const RHI_COMMAND_QUEUE_DESC &queue_desc);
inline void (*rhi_command_queue_execute)(RHI_COMMAND_QUEUE &command_queue, bool wait_completion, fptr_command_queue_on_execute callback);

// command buffer
inline RHI_COMMAND_BUFFER* (*rhi_command_buffer_create_for_copy)(const RHI_COMMAND_BUFFER_DESC &cb_desc);
inline RHI_COMMAND_BUFFER* (*rhi_command_buffer_create_for_compute)(const RHI_COMMAND_BUFFER_DESC &cb_desc);
inline RHI_COMMAND_BUFFER* (*rhi_command_buffer_create_for_render)(const RHI_COMMAND_BUFFER_DESC &cb_desc);
inline void (*rhi_command_buffer_record)(RHI_COMMAND_BUFFER &command_buffer,
										 fptr_command_buffer_on_record callback);
inline void (*rhi_command_buffer_draw_triangle_list)(RHI_COMMAND_BUFFER &command_buffer, RHI_VERTEX_BUFFER &vb, RHI_INDEX_BUFFER*ib);
inline void (*rhi_command_buffer_copy_texture)(RHI_COMMAND_BUFFER& command_buffer, RHI_TEXTURE_2D& dest_texture, RHI_TEXTURE_2D& src_texture);

// buffers
inline RHI_BUFFER* (*rhi_buffers_create_raw)(const RHI_BUFFER_DESC &desc);
inline RHI_VERTEX_BUFFER* (*rhi_vertex_buffer_create)(const RHI_VERTEX_BUFFER_DESC &desc);
inline RHI_INDEX_BUFFER* (*rhi_index_buffer_create)(const RHI_INDEX_BUFFER_DESC &desc);
inline RHI_DEPTH_BUFFER* (*rhi_buffers_create_depth)(const RHI_DEPTH_BUFFER_DESC &desc);
inline RHI_CONSTANT_BUFFER* (*rhi_buffers_create_constant)(const RHI_BUFFER_DESC &desc);
inline RHI_VOID_PTR (*rhi_buffers_map_open)(RHI_BUFFER &shared_buffer, size_t offset,
											size_t length);
inline void (*rhi_buffers_map_close)(RHI_BUFFER &shared_buffer, size_t offset,
									 size_t length);
inline void (*rhi_buffers_gpu_upload_region)(RHI_COMMAND_BUFFER &command_buffer, RHI_BUFFER &shared_buffer,
											 RHI_BUFFER &gpu_buffer, size_t offset_src, size_t offset_dest, size_t length);
inline void (*rhi_buffers_gpu_upload)(RHI_COMMAND_BUFFER &command_buffer, RHI_BUFFER &shared_buffer, RHI_BUFFER &gpu_buffer);
inline void (*rhi_buffers_gpu_download_region)(RHI_COMMAND_BUFFER &command_buffer, RHI_BUFFER &shared_buffer,
											   RHI_BUFFER &gpu_buffer, size_t offset_src, size_t offset_dest, size_t length);
inline void (*rhi_buffers_gpu_download)(RHI_COMMAND_BUFFER &command_buffer, RHI_BUFFER &shared_buffer, RHI_BUFFER &gpu_buffer);
inline void (*rhi_buffers_map_write)(RHI_BUFFER &shared_buffer, RHI_VOID_PTR data,
									 size_t offset, size_t length);
inline void (*rhi_buffers_map_read)(RHI_BUFFER &shared_buffer, RHI_VOID_PTR &data,
									size_t offset, size_t length);

inline void (*rhi_texture_2d_gpu_upload)(RHI_COMMAND_BUFFER& command_buffer, RHI_BUFFER& shared_buffer, RHI_TEXTURE_2D& texture);
inline RHI_TEXTURE_2D* (*rhi_texture_2d_create)(const RHI_TEXTURE_2D_DESC &tex_desc);
inline RHI_SAMPLER*(*rhi_sampler_create)(const RHI_RT_SAMPLER_DESC& desc);

// pipeline
inline RHI_RT_PIPELINE* (*rhi_rt_pipeline_create)(const RHI_RT_PIPELINE_DESC &desc);
inline RHI_RASTER_PIPELINE* (*rhi_raster_pipeline_create)(const RHI_RASTER_PIPELINE_DESC &pipeline_desc);
inline RHI_BUFFER* (*rhi_rt_pipeline_create_sbt)(RHI_DEVICE& device, RHI_RT_SBT_DESC& desc, RHI_RT_PIPELINE& pipeline);

// pipeline layout
inline RHI_PIPELINE_LAYOUT* (*rhi_pipeline_layout_create)(const RHI_PIPELINE_LAYOUT_DESC &desc);
inline RHI_COMPILED_SHADER_BUFFER* (*rhi_shaders_compiler_compile)(const char *const file, const char *const entry, const char *const target);
inline void (*rhi_shaders_compiler_set_folder)(const char *const folder);

// render pass api
inline RHI_RENDER_PASS* (*rhi_render_pass_create)(const RHI_RENDER_PASS_DESC &desc);
inline void (*rhi_render_pass_execute_raster_mode)(RHI_RENDER_PASS &render_pass, RHI_COMMAND_BUFFER &command_buffer,
									   fptr_render_pass_on_execute);
inline void (*rhi_render_pass_execute_rt_mode)(RHI_RENDER_PASS& render_pass, RHI_COMMAND_BUFFER& command_buffer,
fptr_render_pass_on_execute);

// rt
inline RHI_RT_BVH* (*rhi_rt_bvh_create)(const RHI_RT_BVH_DESC& desc);
inline RHI_BUFFER*(*rhi_rt_bvh_build_geometry_instances)(const RT_GEOMETRY_INSTANCES_DESC& desc);
inline void (*rhi_command_buffer_ray_trace)(RHI_DEVICE& device, RHI_COMMAND_BUFFER& command_buffer, RHI_TEXTURE_2D& render_target, RHI_RT_PIPELINE& pipeline, RHI_BUFFER& bvh_instances, RHI_BUFFER& sbt);

#endif