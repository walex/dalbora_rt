#include "dalbora_rt_api.hpp"

void test_create_window() {

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

	// main loop
	rhi_window_main_loop(*window.get());
}