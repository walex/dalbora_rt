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
#include "dx12_rt_pipeline.hpp"
#include "dx12_texture_2d.hpp"
#include "dx12_resource_state.hpp"
#include "dx12_shaders_compiler.hpp"
#include "dx12_render_pass.hpp"
#include "dx12_pipeline_layout.hpp"
#include "dx12_descriptor_pool.hpp"
#include "dx12_buffers.hpp"

void dx12_rhi_init() {

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
	rhi_command_queue_wait_gpu = &dx12_command_queue_wait_gpu;

	// command buffer
	rhi_command_buffer_create = &dx12_command_buffer_create;
	rhi_command_buffer_begin_record = &dx12_command_buffer_begin_record;
	rhi_command_buffer_end_record = &dx12_command_buffer_end_record;

	// pipeline layout
	rhi_descriptor_pool_create = &dx12_descriptor_pool_create;
	rhi_pipeline_layout_create = &dx12_pipeline_layout_create;

	rhi_buffers_upload_synchronized = &dx12_buffers_upload_synchronized;

	rhi_raster_pipeline_create = &dx12_raster_pipeline_create;

	// buffers
	rhi_vertex_buffer_create = &dx12_vertex_buffer_create;
	rhi_index_buffer_create = &dx12_index_buffer_create;
	rhi_buffers_create_depth = &dx12_buffers_create_depth;

	rhi_texture_2d_create = &dx12_texture_2d_create;
	rhi_raster_pipeline_create = &dx12_raster_pipeline_create;
	rhi_rt_pipeline_create = &dx12_rt_pipeline_create;
	rhi_shaders_compiler_compile = &dx12_shaders_compiler_compile;
	rhi_shaders_compiler_set_folder = dx12_shaders_compiler_set_folder;

	rhi_render_pass_create = &dx12_render_pass_create;
	rhi_render_pass_begin = &dx12_render_pass_begin;
	rhi_render_pass_end = &dx12_render_pass_end;
}

void dx12_rhi_end() {
	// destroy factory
	dx12_factory_destroy();
}