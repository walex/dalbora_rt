#include "dx12_rhi.hpp"
#include "dx12_factory.hpp"
#include "dx12_device.hpp"
#include "dx12_command_queue.hpp"
#include "dx12_swap_chain.hpp"
#include "dx12_window.hpp"
#include "dx12_command_buffer.hpp"
#include "dx12_index_buffer.hpp"
#include "dx12_vertex_buffer.hpp"
#include "dx12_raster_pipeline.hpp"
#include "dx12_texture_2d.hpp"
#include "dx12_shaders_compiler.hpp"
#include "dx12_render_pass.hpp"
#include "dx12_pipeline_layout.hpp"
#include "dx12_heap.hpp"
#include "dx12_buffers.hpp"
#include "dx12_sampler.hpp"
#include "dx12_rt_pipeline.hpp"
#include "dx12_rt_bvh.hpp"

void dx12_rhi_init()
{

	// create factory
	dx12_factory_create();

	// set function pointers
	rhi_create_window = &dx12_window_create;
	rhi_window_main_loop = &dx12_window_main_loop;

	// device
	rhi_create_device = &dx12_device_create;

	// swap chain api
	rhi_swap_chain_create = &dx12_swap_chain_create;
	rhi_swap_chain_present = &dx12_swap_chain_present;
	rhi_swap_chain_get_surface = &dx12_swap_chain_get_surface;
	rhi_swap_chain_get_current_buffer_id = &dx12_swap_chain_get_current_buffer_id;

	// command queue
	rhi_command_queue_create_for_render = &dx12_command_queue_create_for_render;
	rhi_command_queue_create_for_compute = &dx12_command_queue_create_for_compute;
	rhi_command_queue_create_for_copy = &dx12_command_queue_create_for_copy;
	rhi_command_queue_execute = &dx12_command_queue_execute;

	// command buffer
	rhi_command_buffer_create_for_copy = &dx12_command_buffer_create_for_copy;
	rhi_command_buffer_create_for_compute = &dx12_command_buffer_create_for_compute;
	rhi_command_buffer_create_for_render = &dx12_command_buffer_create_for_render;
	rhi_command_buffer_record = &dx12_command_buffer_record;
	rhi_command_buffer_ray_trace = &dx12_command_buffer_ray_trace;
	rhi_command_buffer_copy_texture = &dx12_command_buffer_copy_texture;

	rhi_buffers_gpu_upload_region = &dx12_buffers_gpu_upload_region;
	rhi_buffers_gpu_upload = &dx12_buffers_gpu_upload;
	rhi_buffers_map_write = &dx12_buffers_map_write;
	rhi_buffers_map_read = &dx12_buffers_map_read;
	rhi_command_buffer_draw_triangle_list = &dx12_command_buffer_draw_triangle_list;

	// pipeline
	rhi_pipeline_layout_create = &dx12_pipeline_layout_create;
	rhi_raster_pipeline_create = &dx12_raster_pipeline_create;

	// buffers
	rhi_buffers_create_raw = &dx12_buffers_create_raw;
	rhi_vertex_buffer_create = &dx12_vertex_buffer_create;
	rhi_index_buffer_create = &dx12_index_buffer_create;
	rhi_buffers_create_depth = &dx12_buffers_create_depth;
	rhi_buffers_create_constant = &dx12_buffers_create_constant;
	rhi_buffers_map_open = &dx12_buffers_map_open;
	rhi_buffers_map_close = &dx12_buffers_map_close;

	rhi_buffers_gpu_upload_region = &dx12_buffers_gpu_upload_region;
	rhi_buffers_gpu_upload = &dx12_buffers_gpu_upload;
	rhi_buffers_gpu_download_region = &dx12_buffers_gpu_download_region;
	rhi_buffers_gpu_download = &dx12_buffers_gpu_download;
	rhi_buffers_map_write = &dx12_buffers_map_write;
	rhi_buffers_map_read = &dx12_buffers_map_read;
	rhi_texture_2d_gpu_upload = &dx12_texture_2d_gpu_upload;

	rhi_texture_2d_create = &dx12_texture_2d_create;
	rhi_sampler_create = &dx12_sampler_create;
	rhi_raster_pipeline_create = &dx12_raster_pipeline_create;
	rhi_shaders_compiler_compile = &dx12_shaders_compiler_compile;
	rhi_shaders_compiler_set_folder = &dx12_shaders_compiler_set_folder;

	// render pass
	rhi_render_pass_create = &dx12_render_pass_create;
	rhi_render_pass_execute_raster_mode = &dx12_render_pass_execute_raster_mode;
	rhi_render_pass_execute_rt_mode = &dx12_render_pass_execute_rt_mode;

	// rt
	rhi_rt_pipeline_create = &dx12_rt_pipeline_create;
	rhi_rt_bvh_create = &dx12_rt_bvh_create;
	rhi_rt_bvh_build_geometry_instances = &dx12_rt_bvh_build_geometry_instances;
	rhi_rt_pipeline_create_sbt = &dx12_rt_pipeline_create_sbt;

	
}

void dx12_rhi_end()
{
}