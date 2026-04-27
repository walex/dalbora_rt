#include "dx12_rhi.hpp"
#include "dx12_factory.hpp"
#include "dx12_device.hpp"
#include "dx12_command_queue.hpp"
#include "dx12_swap_chain.hpp"
#include "dx12_window.hpp"
#include "dx12_command_buffer.hpp"
#include "dx12_index_buffer.hpp"
#include "dx12_vertex_buffer.hpp"
#include "dx12_pipeline.hpp"
#include "dx12_texture_2d.hpp"
#include "dx12_resource_state.hpp"

void dx12_rhi_init() {

	// create factory
	dx12_create_factory();

	// set function pointers
	rhi_create_device = &dx12_create_device;
	rhi_create_swap_chain = &dx12_create_swap_chain;
	rhi_create_window = &dx12_create_window;
	rhi_create_graphics_command_queue = &dx12_create_graphics_command_queue;
	rhi_create_compute_command_queue = &dx12_create_compute_command_queue;
	rhi_create_transfer_command_queue = &dx12_create_copy_command_queue;
	rhi_create_command_buffer = &dx12_create_command_buffer;
	rhi_create_pipeline = &dx12_create_pipeline;
	rhi_create_vertex_buffer = &dx12_create_vertex_buffer;
	rhi_create_index_buffer = &dx12_create_index_buffer;
	rhi_create_texture_2d = &dx12_create_texture_2d;
}

void dx12_rhi_end() {
	// destroy factory
	dx12_destroy_factory();
}

HWND dx_rhi_get_window(RHI_HANDLE& window) {
	return static_cast<HWND>(reinterpret_cast<DX_WINDOW_HANDLE&>(window));
}


void DX_RHI_RESOURCE::change_state(resource_state new_state) {

	dx12_resource_state_transition(*this->get_command_buffer(), *this, this->get_current_state(), new_state);
	this->set_current_state(new_state);
}
