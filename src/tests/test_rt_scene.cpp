#include "test_api.hpp"

#include "RayTracingRenderer.hpp"
#include "RayTraceScene.hpp"
#include "Camera.hpp"

#define DEGREE_TO_RADIAN(x) ((x) * 3.14159265358979323846 / 180.0)

void test_rt_scene() {

	constexpr size_t SWAP_CHAIN_BUFFER_COUNT = 3;

	std::unique_ptr<RayTracingRenderer> renderer;
	RhiWindow window;
	RhiSwapChain swap_chain;
	RayTraceScene scene;
	RHI_VIEWPORT view_port;
	std::unique_ptr<BaseCamera> camera;

	// TODO: add resource manager
	constexpr size_t read_only_shader_registers_count = 800;
	constexpr size_t rw_shader_registers_count = 1;
	constexpr size_t constant_shader_registers_count = 1;

	RHI_WINDOW_CALLBACKS window_callbacks;
	window_callbacks.on_init = ([&](RHI_WINDOW* const wnd) {

		// create render and swap chain
		resource_format surface_format = resource_format_R8G8B8A8_norm;
		renderer = std::make_unique<RayTracingRenderer>(surface_format, window.get_width(),
			window.get_height(), read_only_shader_registers_count,
			rw_shader_registers_count, constant_shader_registers_count);
		swap_chain = renderer->create_swap_chain(window, SWAP_CHAIN_BUFFER_COUNT, surface_format);
		view_port.x = 0;
		view_port.y = 0;
		view_port.width = static_cast<float>(window.get_width());
		view_port.height = static_cast<float>(window.get_height());
		view_port.min_z = 0.0f;
		view_port.max_z = 1.0f;

		// load scene from file
		std::filesystem::path model_3d_folder = get_executable_folder("test_3d_models");
		std::string model_3d_file = (model_3d_folder / "scene.gltf").string();
		if (std::filesystem::exists(model_3d_file) == false) {
			throw std::exception("3d model file deos not exists");
		}
		scene.initialize(renderer->get_device(), surface_format, 
			read_only_shader_registers_count, rw_shader_registers_count, 
			constant_shader_registers_count);
		scene.set_max_size(6 * 1024 * 1024);
		scene.load(model_3d_file, renderer->get_device(), 
			renderer->get_command_queue());

		// camera config
		camera = std::make_unique<BaseCamera>(renderer->get_device());
		float3 bb_min = scene.get_bb_min();
		float3 bb_max = scene.get_bb_max();
		float3 camera_center = float3((bb_min + bb_max) * 0.5f);
		float3 size = bb_max - bb_min;
		float max_dimension =
			std::max({
				size.x,
				size.y,
				size.z
				});
		float3 camera_pos =
			camera_center + float3(
				0.0f,
				0.0f,
				max_dimension * -0.09f);

		camera->set_look_at(camera_center - camera_pos);
		camera->set_position(camera_pos);
		camera->set_up(float3(0.0f, 1.0f, 0.0f));
		camera->set_right(float3(1.0f, 0.0f, 0.0f));
		camera->set_fov(DEGREE_TO_RADIAN(45.0f));
		camera->set_aspect(static_cast<float>(window.get_width()) / static_cast<float>(window.get_height()));
		scene.set_camera(camera.get());
	});

	window_callbacks.on_end = ([&](RHI_WINDOW* const wnd) {

	});

	window_callbacks.on_idle = ([&](RHI_WINDOW* const wnd) {
		
		RhiView render_target_view = swap_chain.get_next_render_target();
		scene.draw_scene(*renderer, render_target_view, view_port);
		swap_chain.present();
	});
	
	window.create("Ray Tracing Scene Test", 800, 600, false, window_callbacks);
	window.mainLoop();

}