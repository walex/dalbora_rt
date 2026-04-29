#include "test_api.hpp"

void test_create_window(const RHI_WINDOW_CALLBACKS* const callbacks) {

	// setup window
	RHI_WINDOW_DESC window_desc;
	window_desc.full_screen = false;
	window_desc.width = 800;
	window_desc.height = 600;	
	if (callbacks != nullptr) {
		window_desc.callbacks = *callbacks;
	} 

	if (window_desc.callbacks.on_init == nullptr) {
	
		window_desc.callbacks.on_init = ([](RHI_OBJECT&) {
			printf("Window initialized.\n");
			});
	}

	if (window_desc.callbacks.main_loop == nullptr) {
	
		window_desc.callbacks.main_loop = ([]() {
			static auto last_print = std::chrono::steady_clock::now();
			auto diff = std::chrono::steady_clock::now() - last_print;
			if (std::chrono::duration_cast<std::chrono::seconds>(diff).count() >= 1) {
				printf("Window loop running...\n");
				last_print = std::chrono::steady_clock::now();
			}
			});
	}
	
	if (window_desc.callbacks.on_end == nullptr) {
	
		window_desc.callbacks.on_end = ([]() {
			printf("Window terminated.\n");
			});
	}

	strcpy_s(window_desc.title, "pbr test create window");

	// create window
	auto window = rhi_create_window(window_desc);

	// main loop
	rhi_window_main_loop(*window.get());
}