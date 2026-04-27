#include "dx12_hal.hpp"
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

void dx12_hal_init() {

	// create factory
	dx12_create_factory();

	// set function pointers
	hal_create_device = &dx12_create_device;
	hal_create_swap_chain = &dx12_create_swap_chain;
	hal_create_window = &dx12_create_window;
	hal_create_graphics_command_queue = &dx12_create_graphics_command_queue;
	hal_create_compute_command_queue = &dx12_create_compute_command_queue;
	hal_create_transfer_command_queue = &dx12_create_copy_command_queue;
	hal_create_command_buffer = &dx12_create_command_buffer;
	hal_create_pipeline = &dx12_create_pipeline;
	hal_create_vertex_buffer = &dx12_create_vertex_buffer;
	hal_create_index_buffer = &dx12_create_index_buffer;
	hal_create_texture_2d = &dx12_create_texture_2d;
}

void dx12_hal_end() {
	// destroy factory
	dx12_destroy_factory();
}

HWND dx_hal_get_window(HAL_HANDLE& window) {
	return static_cast<HWND>(reinterpret_cast<DX_WINDOW_HANDLE&>(window));
}


void DX_HAL_RESOURCE::change_state(resource_state new_state) {

	dx12_resource_state_transition(*this->get_command_buffer(), *this, this->get_current_state(), new_state);
	this->set_current_state(new_state);
}
