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
#include "dx12_shaders.hpp"
#include "dx12_render_pass.hpp"

void dx12_rhi_init() {

	// create factory
	dx12_factory_create();

	// set function pointers
	rhi_create_window = &dx12_window_create;
	rhi_window_main_loop = &dx12_window_main_loop;

	rhi_create_device = &dx12_device_create;

	// swap chain api
	rhi_swap_chain_create = &dx12_swap_chain_create;
	rhi_swap_chain_present = &dx12_swap_chain_present;
	rhi_swap_chain_get_surface = &dx12_swap_chain_get_surface;
	rhi_swap_chain_get_current_buffer_id = &dx12_swap_chain_get_current_buffer_id;

	rhi_command_queue_create_for_render = &dx12_command_queue_create_for_render;
	rhi_command_queue_create_for_compute = &dx12_command_queue_create_for_compute;
	rhi_command_queue_create_for_copy = &dx12_command_queue_create_for_copy;
	rhi_command_buffer_create = &dx12_command_buffer_create;
	rhi_raster_pipeline_create = &dx12_raster_pipeline_create;
	rhi_vertex_buffer_create = &dx12_vertex_buffer_create;
	rhi_index_buffer_create = &dx12_index_buffer_create;
	rhi_texture_2d_create = &dx12_texture_2d_create;
	rhi_raster_pipeline_create = &dx12_raster_pipeline_create;
	rhi_rt_pipeline_create = &dx12_rt_pipeline_create;
	rhi_shaders_compile = &dx12_shaders_compile;

	rhi_render_pass_create = &dx12_render_pass_create;
	rhi_render_pass_begin = &dx12_render_pass_begin;
	rhi_render_pass_end = &dx12_render_pass_end;
}

void dx12_rhi_end() {
	// destroy factory
	dx12_factory_destroy();
}