#include "dalbora_rt_api.hpp"

void test_empty_device() {

	// init rhi
	rhi_init(device_type_dx12);

	// setup window
	RHI_WINDOW_DESC window_desc;
	window_desc.full_screen = false;
	window_desc.width = 800;
	window_desc.height = 600;
	window_desc.callback = []() {
		static auto last_print = std::chrono::steady_clock::now();
		auto diff = std::chrono::steady_clock::now() - last_print;
		if (std::chrono::duration_cast<std::chrono::seconds>(diff).count() >= 1) {
			printf("Windows loop running...\n");
			last_print = std::chrono::steady_clock::now();
		}
		};
	strcpy_s(window_desc.title, "pbr test create window");
	
	// create window
	auto window = rhi_create_window(window_desc);

	RHI_DEVICE_DESC device_desc;
	device_desc.adapter_id = 0;
	device_desc.features = device_features_raytracing;
	auto dev = rhi_create_device(device_desc);

	RHI_COMMAND_QUEUE_DESC queue_desc;
	queue_desc.device = dev.get();
	auto queue = rhi_create_graphics_command_queue(queue_desc);

	RHI_SWAP_CHAIN_DESC swap_chain_desc;
	swap_chain_desc.width = 800;
	swap_chain_desc.height = 600;
	swap_chain_desc.allow_tearing = false;
	swap_chain_desc.buffer_count = 2;
	swap_chain_desc.color_format = resource_format_R8G8B8A8;
	swap_chain_desc.device = dev.get();
	swap_chain_desc.command_queue = queue.get();
	swap_chain_desc.window = window.get();
	auto swap_chain = rhi_create_swap_chain(swap_chain_desc);

	// main loop
	rhi_window_main_loop(*window.get());
}

