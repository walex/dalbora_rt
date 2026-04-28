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

void dx12_rhi_init() {

	// create factory
	dx12_create_factory();

	// set function pointers
	rhi_create_window = &dx12_window_create;
	rhi_window_main_loop = &dx12_window_main_loop;

	rhi_create_device = &dx12_create_device;
	rhi_create_swap_chain = &dx12_create_swap_chain;
	rhi_create_graphics_command_queue = &dx12_create_graphics_command_queue;
	rhi_create_compute_command_queue = &dx12_create_compute_command_queue;
	rhi_create_transfer_command_queue = &dx12_create_copy_command_queue;
	rhi_create_command_buffer = &dx12_create_command_buffer;
	rhi_create_pipeline = &dx12_create_raster_pipeline;
	rhi_create_vertex_buffer = &dx12_vertex_buffer_create;
	rhi_create_index_buffer = &dx12_index_buffer_create;
	rhi_create_texture_2d = &dx12_create_texture_2d;
	rhi_create_raster_pipeline = &dx12_create_raster_pipeline;
	rhi_create_rt_pipeline = &dx12_create_rt_pipeline;
	rhi_compile_shader = &dx12_shaders_compile;
}

void dx12_rhi_end() {
	// destroy factory
	dx12_destroy_factory();
}