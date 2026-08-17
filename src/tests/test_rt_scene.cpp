#include "test_api.hpp"

#include "RayTracingRenderer.hpp"

void test_rt_scene() {

	constexpr size_t SWAP_CHAIN_BUFFER_COUNT = 3;

	std::unique_ptr<RayTracingRenderer> renderer;
	RhiWindow window;
	RhiSwapChain swap_chain;
	RHI_VIEWPORT view_port;

	RHI_WINDOW_CALLBACKS callbacks;
	callbacks.on_init = ([&](RHI_WINDOW* const wnd) {

		renderer = std::make_unique<RayTracingRenderer>();
		swap_chain = renderer->create_swap_chain(window, SWAP_CHAIN_BUFFER_COUNT);
		view_port.x = 0;
		view_port.y = 0;
		view_port.width = static_cast<float>(window.get_width());
		view_port.height = static_cast<float>(window.get_height());
		view_port.min_z = 0.0f;
		view_port.max_z = 1.0f;
	});

	callbacks.on_end = ([&](RHI_WINDOW* const wnd) {

	});

	callbacks.on_idle = ([&](RHI_WINDOW* const wnd) {
		
		RhiView render_target_view = swap_chain.get_next_render_target();
		renderer->draw(render_target_view, view_port);
		swap_chain.present();
	});
	
	window.create("Ray Tracing Scene Test", 800, 600, false, callbacks);
	window.mainLoop();

}