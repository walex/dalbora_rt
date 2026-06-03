#include "test_api.hpp"

#ifdef TEST_WINDOW

void test_create_window(std::shared_ptr<RHI_WINDOW_CALLBACKS> callbacks) {

	// setup window
	RHI_WINDOW_DESC window_desc;
	window_desc.full_screen = false;
	window_desc.width = 800;
	window_desc.height = 600;	
	std::unique_ptr<RHI_WINDOW_CALLBACKS> cbs;
	if (callbacks.get() != nullptr) {
		window_desc.callbacks = callbacks.get();
	}
	else {
		cbs = std::make_unique<RHI_WINDOW_CALLBACKS>();
		window_desc.callbacks = cbs.get();
	}

	if (window_desc.callbacks->on_init == nullptr) {
	
		window_desc.callbacks->on_init = ([&](const RHI_WINDOW* const UNUSED_PARAM(window)) {
			printf("Window initialized.\n");
			});
	}

	if (window_desc.callbacks->main_loop == nullptr) {
	
		window_desc.callbacks->main_loop = ([&](const RHI_WINDOW* const UNUSED_PARAM(window)) {
			static auto last_print = std::chrono::steady_clock::now();
			auto diff = std::chrono::steady_clock::now() - last_print;
			if (std::chrono::duration_cast<std::chrono::seconds>(diff).count() >= 1) {
				printf("Window loop running...\n");
				last_print = std::chrono::steady_clock::now();
			}
			});
	}
	
	if (window_desc.callbacks->on_end == nullptr) {
	
		window_desc.callbacks->on_end = ([&](const RHI_WINDOW* const UNUSED_PARAM(window)) {
			printf("Window terminated.\n");
			});
	}

	const char* title = "pbr test for create window";
	window_desc.title = const_cast<char*>(title);

	// create window
	std::unique_ptr<RHI_WINDOW> window(rhi_create_window(&window_desc));

	// main loop
	rhi_window_main_loop(window.get());
}

void test_create_window_obj(RhiUnitTestCallbacks* callbacks) {

	RhiUnitTest unit_test;
	RHI_WINDOW_CALLBACKS window_callbacks;
	window_callbacks.on_init = [&](RHI_WINDOW* hwindow) {
		
		unit_test.window = RhiWindow(hwindow);
		if (callbacks)
			callbacks->on_init(unit_test);
	};
	window_callbacks.main_loop = [&](RHI_WINDOW* hwindow) {
		
		if (callbacks)
			callbacks->on_process(unit_test);
	};
	window_callbacks.on_end = [&](RHI_WINDOW* hwindow) {
		
		if (callbacks)
			callbacks->on_end(unit_test);
	};

	RhiWindow wnd;
	wnd.create("test", 800, 600, false, window_callbacks);
	wnd.mainLoop();
}
#endif