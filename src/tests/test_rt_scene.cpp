#include "test_api.hpp"

#include "RayTracingRenderer.hpp"
#include "RayTraceScene.hpp"
#include "Camera.hpp"
#include "ResourceManager.hpp"

#define DEGREE_TO_RADIAN(x) ((x) * 3.14159265358979323846 / 180.0)

std::unique_ptr<BaseCamera> create_camera(ResourceManager& resource_manager, float3 bb_min,
	float3 bb_max, float aspect) {

	std::unique_ptr<BaseCamera> camera = std::make_unique<BaseCamera>(resource_manager);
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
	camera->set_aspect(aspect);
	return camera;
}

void test_rt_scene() {
	
	std::unique_ptr<ResourceManager> resources_manager;
	{
		// bindless table for buffers
		constexpr size_t read_only_shader_count = 800;
		constexpr size_t rw_shader_count = 10;
		constexpr size_t constant_shader_count = 1;

		RHI_VIEWPORT view_port;

		RhiWindow window;
		RhiDevice device;
		RhiSwapChain swap_chain;

		std::unique_ptr<RayTracingRenderer> renderer;
		std::unique_ptr<RayTraceScene> scene;
		std::unique_ptr<BaseCamera> camera;

		// windows callbacks
		RHI_WINDOW_CALLBACKS window_callbacks;
		window_callbacks.on_init = ([&](RHI_WINDOW* const wnd) {

			// view port
			view_port.x = 0;
			view_port.y = 0;
			view_port.width = static_cast<float>(window.get_width());
			view_port.height = static_cast<float>(window.get_height());
			view_port.min_z = 0.0f;
			view_port.max_z = 1.0f;

			// create device
			RHI_DEVICE_DESC device_desc;
			device_desc.features |= device_features_raytracing;
			device_desc.shader_model = hlsl_shader_model_6_8;
			device_desc.app_instance = rhi_get_app_instance();
			device.create(device_desc);

			// create resource manager
			resources_manager = std::make_unique<ResourceManager>(device);
			// we use descriptor tables
			resources_manager->create_descriptor_tables(constant_shader_count, read_only_shader_count,
				rw_shader_count);

			// create renderer
			resource_format surface_format = resource_format_R8G8B8A8_norm;
			renderer = std::make_unique<RayTracingRenderer>(*resources_manager, surface_format,
				window.get_width(), window.get_height());

			// create swap chain
			bool enable_vertical_sync;
#ifdef DEBUG
			enable_vertical_sync = true;
#else
			enable_vertical_sync = false;
#endif
			constexpr size_t swap_chain_buffers_count = 3;
			swap_chain = renderer->create_swap_chain(window, swap_chain_buffers_count,
				surface_format, enable_vertical_sync);

			// load scene from file
			std::filesystem::path model_3d_folder = get_executable_folder("test_3d_models");
			std::string model_3d_file = (model_3d_folder / "scene.gltf").string();
			if (std::filesystem::exists(model_3d_file) == false) {
				throw std::exception("3d model file deos not exists");
			}
			scene = std::make_unique<RayTraceScene>(*resources_manager, surface_format);
			scene->set_max_size(6 * 1024 * 1024);
			scene->load(model_3d_file, renderer->get_command_queue());

			// create camera
			float3 bb_min = scene->get_bb_min();
			float3 bb_max = scene->get_bb_max();
			float aspect = static_cast<float>(window.get_width()) / static_cast<float>(window.get_height());
			camera = create_camera(*resources_manager, bb_min, bb_max, aspect);
			scene->set_camera(camera.get());
			});

		window_callbacks.on_end = ([&](RHI_WINDOW* const wnd) {
			});

		window_callbacks.on_idle = ([&](RHI_WINDOW* const wnd) {

			// get elapsed time
			float dt = get_delta_time();

			// update camera
			camera->update(dt);

			// get next render target view from swap chain
			RhiView render_surface = swap_chain.get_next_render_target();

			scene->draw(*renderer, render_surface, view_port);

			// present swap chain
			swap_chain.present();

			// log fps
			print_fps();

			});

		window.create("Ray Tracing Scene Test", 800, 600, false, window_callbacks);
		window.mainLoop();
	}
}