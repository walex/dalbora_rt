#include "test_api.hpp"

void test_create_window(std::shared_ptr<RHI_WINDOW_CALLBACKS> callbacks) {

	// setup window
	RHI_WINDOW_DESC window_desc;
	window_desc.full_screen = false;
	window_desc.width = 800;
	window_desc.height = 600;	
	if (callbacks.get() != nullptr) {
		window_desc.callbacks = callbacks;
	}
	else {
		window_desc.callbacks = std::make_shared<RHI_WINDOW_CALLBACKS>();
	}

	if (window_desc.callbacks.get()->on_init == nullptr) {
	
		window_desc.callbacks.get()->on_init = ([&](RHI_WINDOW& UNUSED_PARAM(window)) {
			printf("Window initialized.\n");
			});
	}

	if (window_desc.callbacks.get()->main_loop == nullptr) {
	
		window_desc.callbacks.get()->main_loop = ([&](RHI_WINDOW& UNUSED_PARAM(window)) {
			static auto last_print = std::chrono::steady_clock::now();
			auto diff = std::chrono::steady_clock::now() - last_print;
			if (std::chrono::duration_cast<std::chrono::seconds>(diff).count() >= 1) {
				printf("Window loop running...\n");
				last_print = std::chrono::steady_clock::now();
			}
			});
	}
	
	if (window_desc.callbacks.get()->on_end == nullptr) {
	
		window_desc.callbacks.get()->on_end = ([&](RHI_WINDOW& UNUSED_PARAM(window)) {
			printf("Window terminated.\n");
			});
	}

	strcpy_s(window_desc.title, "pbr test create window");

	// create window
	std::unique_ptr<RHI_WINDOW> window(rhi_create_window(&window_desc));

	// main loop
	rhi_window_main_loop(window.get());
}